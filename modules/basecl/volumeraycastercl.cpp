/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include "volumeraycastercl.h"
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/kernelmanager.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h> // To check if the we can use sharing
#include <modules/opencl/syncclgl.h>
#include <modules/opencl/volume/volumecl.h>
#include <modules/opencl/volume/volumeclgl.h>

namespace inviwo {

VolumeRaycasterCL::VolumeRaycasterCL()
    : KernelOwner()
    , workGroupSize_(svec2(8, 8))
    , useGLSharing_(true)
    , outputOffset_(0)
    , outputSize_(1)
    , camera_(NULL)
    , samplingRate_(2.f)
    , lightStruct_(sizeof(utilcl::LightParameters), DataUINT8::get(), BufferType::POSITION_ATTRIB, BufferUsage::STATIC, NULL, CL_MEM_READ_ONLY)
    , kernel_(NULL)
{
    light_.ambientColor =vec4(1.f); light_.diffuseColor =vec4(1.f); light_.specularColor =vec4(1.f);
    light_.specularExponent = 110.f; light_.position = vec4(0.7f); light_.shadingMode = ShadingMode::Phong;

    compileKernel();
}

VolumeRaycasterCL::~VolumeRaycasterCL() {}


void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const Image* entryPoints, const Image* exitPoints, const Layer* transferFunction, Image* outImage, const VECTOR_CLASS<cl::Event> *waitForEvents /*= NULL*/, cl::Event *event /*= NULL*/) {

    svec2 localWorkGroupSize(workGroupSize_);
    svec2 globalWorkGroupSize(getGlobalWorkGroupSize(outputSize_.x, localWorkGroupSize.x), getGlobalWorkGroupSize(outputSize_.y,
        localWorkGroupSize.y));

    try {
        // This macro will create an event called profilingEvent if IVW_PROFILING is enabled,
        // otherwise the profilingEvent will be declared as a null pointer
        IVW_OPENCL_PROFILING(profilingEvent, "")

            if (useGLSharing_) {
                // SyncCLGL will synchronize with OpenGL upon creation and destruction
                SyncCLGL glSync;
                const ImageCLGL* entryCL = entryPoints->getRepresentation<ImageCLGL>();
                const ImageCLGL* exitCL = exitPoints->getRepresentation<ImageCLGL>();
                ImageCLGL* outImageCL = outImage->getEditableRepresentation<ImageCLGL>();
                const VolumeCLGL* volumeCL = volume->getRepresentation<VolumeCLGL>();
                const LayerCLGL* transferFunctionCL = transferFunction->getRepresentation<LayerCLGL>();
                // Shared objects must be acquired before use.
                glSync.addToAquireGLObjectList(entryCL);
                glSync.addToAquireGLObjectList(exitCL);
                glSync.addToAquireGLObjectList(outImageCL);
                glSync.addToAquireGLObjectList(volumeCL);
                glSync.addToAquireGLObjectList(transferFunctionCL);
                // Acquire all of the objects at once
                glSync.aquireAllObjects();

                volumeRaycast(volume, volumeCL, entryCL->getLayerCL(), exitCL->getLayerCL(), transferFunctionCL, outImageCL->getLayerCL(), globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
            } else {
                const ImageCL* entryCL = entryPoints->getRepresentation<ImageCL>();
                const ImageCL* exitCL = exitPoints->getRepresentation<ImageCL>();
                ImageCL* outImageCL = outImage->getEditableRepresentation<ImageCL>();
                const VolumeCL* volumeCL = volume->getRepresentation<VolumeCL>();
                const LayerCL* transferFunctionCL = transferFunction->getRepresentation<LayerCL>();

                volumeRaycast(volume, volumeCL, entryCL->getLayerCL(), exitCL->getLayerCL(), transferFunctionCL, outImageCL->getLayerCL(), globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
            }

            // This macro will destroy the profiling event and print the timing in the console if IVW_PROFILING is enabled

    } catch (cl::Error& err) {
        LogError(getCLErrorString(err));
    }
}


void VolumeRaycasterCL::volumeRaycast(const Volume* volume, const VolumeCLBase* volumeCL, const LayerCLBase* entryCLGL, const LayerCLBase* exitCLGL, const LayerCLBase* transferFunctionCL, LayerCLBase* outImageCL, svec2 globalWorkGroupSize, svec2 localWorkGroupSize, const VECTOR_CLASS<cl::Event> *waitForEvents /*= NULL*/, cl::Event *event /*= NULL*/) {
    // Note that the overloaded setArg methods requires 
    // the reference to an object (not the pointer), 
    // which is why we need to dereference the pointers
    kernel_->setArg(0, *volumeCL);
    kernel_->setArg(1, *(volumeCL->getVolumeStruct(volume).getRepresentation<BufferCL>())); // Scaling for 12-bit data
    kernel_->setArg(2, *entryCLGL);
    kernel_->setArg(3, *exitCLGL);
    kernel_->setArg(4, *transferFunctionCL);
    kernel_->setArg(5, camera_->getLookFrom());
    kernel_->setArg(8, *outImageCL);
    //
    OpenCL::getPtr()->getQueue().enqueueNDRangeKernel(*kernel_, cl::NullRange, globalWorkGroupSize, localWorkGroupSize, waitForEvents, event);
}

void VolumeRaycasterCL::samplingRate(float samplingRate) {
    samplingRate_ = samplingRate;
    if (kernel_) {
        try {
            kernel_->setArg(6, samplingRate);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }
}

void VolumeRaycasterCL::outputOffset(ivec2 val) {
    if (kernel_) {
        try {
            kernel_->setArg(9, val);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }
        
    outputOffset_ = val;
}

void VolumeRaycasterCL::outputSize(ivec2 val) {
    if (kernel_) {
        try {
            kernel_->setArg(10, val);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }
        
    outputSize_ = val;
}

void VolumeRaycasterCL::setLightingProperties(ShadingMode::Modes mode, vec3 lightPosition, const vec3& ambientColor, const vec3& diffuseColor, const vec3& specularColor, int specularExponent) {
    light_.position.xyz = lightPosition;
    light_.ambientColor.xyz = ambientColor;
    light_.diffuseColor.xyz = diffuseColor;
    light_.specularColor.xyz = specularColor;
    light_.specularExponent = specularExponent;
    if (mode != light_.shadingMode) {
        light_.shadingMode = mode;
        compileKernel();
    }
    if (kernel_) {
        try {
            // Update data before returning it
            lightStruct_.upload(&light_, sizeof(utilcl::LightParameters));
            
            kernel_->setArg(7, lightStruct_);
        } catch (cl::Error& err) {
            LogError(getCLErrorString(err));
        }
    }

}

void VolumeRaycasterCL::setLightingProperties(const SimpleLightingProperty& light) {
    setLightingProperties(ShadingMode::Modes(light.shadingMode_.get()), light.lightPosition_.get(), light.ambientColor_.get(), light.diffuseColor_.get(), light.specularColor_.get(), light.specularExponent_.get());
}

void VolumeRaycasterCL::compileKernel() {

    if (kernel_) {
        removeKernel(kernel_);
    }
    std::stringstream defines;
    if (light_.shadingMode != 0)
        defines << " -D SHADING_MODE=" << light_.shadingMode;
    // Will compile kernel and make sure that it it
    // recompiled whenever the file changes
    // If the kernel fails to compile it will be set to NULL
    kernel_ = addKernel("volumeraycaster.cl", "raycaster", defines.str());
    // Update kernel arguments that are only set once they are changed
    outputOffset(outputOffset_);
    outputSize(outputSize_);
    samplingRate(samplingRate());
    setLightingProperties(light_.shadingMode, light_.position.xyz, light_.ambientColor.xyz, light_.diffuseColor.xyz, light_.specularColor.xyz, light_.specularExponent);
}

} // namespace
