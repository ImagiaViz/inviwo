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

#ifndef IVW_VOLUME_RAYCASTER_CL_H
#define IVW_VOLUME_RAYCASTER_CL_H

#include <modules/basecl/baseclmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/cameraproperty.h>

#include <modules/opencl/inviwoopencl.h>
#include <modules/opencl/buffer/buffercl.h>
#include <modules/opencl/image/layerclbase.h>
#include <modules/opencl/kernelowner.h>
#include <modules/opencl/utilcl.h>
#include <modules/opencl/volume/volumeclbase.h>

namespace inviwo {

/**
 * \brief Perform volume rendering on the input volume. 
 *
 */
class IVW_MODULE_BASECL_API VolumeRaycasterCL : public KernelOwner {
public:
    VolumeRaycasterCL();
    ~VolumeRaycasterCL();

    bool isValid() const { return kernel_ != NULL; }



    /** 
     * \brief Perform volume rendering on the input volume. 
     * 
     * @param const Volume * volume 
     * @param const Image * entryPoints Start point of ray in texture space.
     * @param const Image * exitPoints End point of ray in texture space.
     * @param const Layer * transferFunction Transfer function, mapping value to color and opacity.
     * @param Image * outImage Output image 
     * @param const VECTOR_CLASS<cl::Event> * waitForEvents 
     * @param cl::Event * event 
     */
    void volumeRaycast(const Volume* volume, const Image* entryPoints, const Image* exitPoints, const Layer* transferFunction, Image* outImage, const VECTOR_CLASS<cl::Event> *waitForEvents = NULL, cl::Event *event = NULL);
    
    void volumeRaycast(const Volume* volume, const VolumeCLBase* volumeCL, const LayerCLBase* entryCLGL, const LayerCLBase* exitCLGL, const LayerCLBase* transferFunctionCL, LayerCLBase* outImageCL, svec2 globalWorkGroupSize, svec2 localWorkGroupSize, const VECTOR_CLASS<cl::Event> *waitForEvents = NULL, cl::Event *event = NULL);
    
    void samplingRate(float samplingRate);
    float samplingRate() const { return samplingRate_; }

    void setCamera(CameraProperty* camera) { camera_ = camera; }
    void setLightingProperties(const SimpleLightingProperty& light);
    void setLightingProperties(ShadingMode::Modes mode, vec3 lightPosition, const vec3& ambientColor, const vec3& diffuseColor, const vec3& specularColor, int specularExponent);

    svec2 workGroupSize() const { return workGroupSize_; }
    void workGroupSize(const svec2& val) { workGroupSize_ = val; }

    bool useGLSharing() const { return useGLSharing_; }
    void useGLSharing(bool val) { useGLSharing_ = val; }

    ivec2 outputOffset() const { return outputOffset_; }
    void outputOffset(ivec2 val);
    ivec2 outputSize() const { return outputSize_; }
    void outputSize(ivec2 val);
private:
    void compileKernel();
    // Parameters
    svec2 workGroupSize_;
    bool useGLSharing_;
    ivec2 outputOffset_;
    ivec2 outputSize_;
    CameraProperty* camera_;
    utilcl::LightParameters light_;
    float samplingRate_;

    BufferCL lightStruct_;
    cl::Kernel* kernel_;
};

} // namespace

#endif // IVW_VOLUME_RAYCASTER_CL_H
