/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2013-2014 Inviwo Foundation
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
 * Contact: Erik Sund�n
 *
 *********************************************************************************/

#include "geometryrenderprocessorgl.h"
#include <modules/opengl/geometry/geometrygl.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/rendering/geometryrendererfactory.h>
#include <modules/opengl/rendering/meshrenderer.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/opengl/textureutils.h>
#include <modules/opengl/shaderutils.h>

namespace inviwo {

ProcessorClassIdentifier(GeometryRenderProcessorGL, "org.inviwo.GeometryRenderGL");
ProcessorDisplayName(GeometryRenderProcessorGL,  "Geometry Renderer");
ProcessorTags(GeometryRenderProcessorGL, Tags::GL);
ProcessorCategory(GeometryRenderProcessorGL, "Geometry Rendering");
ProcessorCodeState(GeometryRenderProcessorGL, CODE_STATE_STABLE);

GeometryRenderProcessorGL::GeometryRenderProcessorGL()
    : Processor()
    , inport_("geometry.inport")
    , outport_("image.outport")
    , camera_("camera", "Camera")
    , centerViewOnGeometry_("centerView", "Center view on geometry")
    , resetViewParams_("resetView", "Reset Camera")
    , trackball_(NULL)
    , geomProperties_("geometry", "Geometry Rendering Properties")
    , cullFace_("cullFace", "Cull Face")
    , polygonMode_("polygonMode", "Polygon Mode")
    , renderPointSize_("renderPointSize", "Point Size", 1.0f, 0.001f, 15.0f, 0.001f)
    , lightingProperty_("lighting", "Lighting")
{
    
    addPort(inport_);
    addPort(outport_);
    addProperty(camera_);
    trackball_ = new CameraTrackball(&camera_);
    addInteractionHandler(trackball_);
    centerViewOnGeometry_.onChange(this, &GeometryRenderProcessorGL::centerViewOnGeometry);
    addProperty(centerViewOnGeometry_);
    resetViewParams_.onChange(this, &GeometryRenderProcessorGL::resetViewParams);
    addProperty(resetViewParams_);
    outport_.addResizeEventListener(&camera_);
    inport_.onChange(this,&GeometryRenderProcessorGL::updateRenderers);

    cullFace_.addOption("culldisable", "Disable", 0);
    cullFace_.addOption("cullfront", "Front", GL_FRONT);
    cullFace_.addOption("cullback", "Back", GL_BACK);
    cullFace_.addOption("cullfrontback", "Front & Back", GL_FRONT_AND_BACK);
    cullFace_.set(0);

    polygonMode_.addOption("polypoint", "Points", GL_POINT);
    polygonMode_.addOption("polyline", "Lines", GL_LINE);
    polygonMode_.addOption("polyfill", "Fill", GL_FILL);
    polygonMode_.set(GL_FILL);

    geomProperties_.addProperty(cullFace_);
    geomProperties_.addProperty(polygonMode_);
    geomProperties_.addProperty(renderPointSize_);

    addProperty(geomProperties_);
    addProperty(lightingProperty_);

    setAllPropertiesCurrentStateAsDefault();
}

GeometryRenderProcessorGL::~GeometryRenderProcessorGL() {
    removeInteractionHandler(trackball_);
    delete trackball_;
    trackball_ = NULL;
}

void GeometryRenderProcessorGL::initialize() {
    Processor::initialize();
    shader_ = new Shader("geometryrendering.vert", "geometryrendering.frag", false);
    initializeResources();
}

void GeometryRenderProcessorGL::deinitialize() {
    // Delete all renderers
    for (std::vector<GeometryRenderer*>::iterator it = renderers_.begin(), endIt = renderers_.end(); it != endIt; ++it) {
        delete *it;
    }
    if (shader_) 
        delete shader_;
    shader_ = NULL;
    Processor::deinitialize();
}

void GeometryRenderProcessorGL::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);
    shader_->build();
}

void GeometryRenderProcessorGL::process() {
    if (!inport_.hasData()) {
        return;
    }

    GLint prevPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode_.get());
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    if (!depthTest) {    
        glEnable(GL_DEPTH_TEST);
    }

    utilgl::activateAndClearTarget(outport_);

    shader_->activate();
    setGlobalShaderParameters(shader_);
    utilgl::setShaderUniforms(shader_, camera_, "camera_");
    utilgl::setShaderUniforms(shader_, lightingProperty_, "lighting_");

    bool culling = (cullFace_.get() != 0);
    if (culling) {
        glEnable(GL_CULL_FACE); 
        glCullFace(cullFace_.get());
    }

    if (polygonMode_.get()==GL_LINE) {
        // FIX: disabled line smoothing to avoid blending artifacts with background (issue #611)
        //glEnable(GL_LINE_SMOOTH);
        glLineWidth((GLfloat)renderPointSize_.get());
    }
    else if (polygonMode_.get()==GL_POINT)
        glPointSize((GLfloat)renderPointSize_.get());

    for (std::vector<GeometryRenderer*>::const_iterator it = renderers_.begin(), endIt = renderers_.end(); it != endIt; ++it) {
        //utilgl::setShaderUniforms(shader_, camera_, *(*it)->getGeometry());
        utilgl::setShaderUniforms(shader_, *(*it)->getGeometry(), "geometry_");
        shader_->setUniform("viewToTexture_", camera_.inverseViewMatrix()*(*it)->getGeometry()->getCoordinateTransformer().getWorldToModelMatrix());
        mat4 modelViewMatrix = camera_.viewMatrix()*(*it)->getGeometry()->getWorldTransform()*(*it)->getGeometry()->getBasisAndOffset();
        shader_->setUniform("modelViewMatrix_", modelViewMatrix);
        (*it)->render();
    }

    if (polygonMode_.get()==GL_LINE) {
        // FIX: disabled line smoothing to avoid blending artifacts with background (issue #611)
        //glDisable(GL_LINE_SMOOTH);
    }

    shader_->deactivate();

    utilgl::deactivateCurrentTarget();

    if (culling) {
        glDisable(GL_CULL_FACE);
    }
    if (!depthTest) {
        glDisable(GL_DEPTH_TEST);
    }
    // restore
    glPointSize(1.f);
    glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
}

void GeometryRenderProcessorGL::centerViewOnGeometry() {
    std::vector<const Geometry*> geometries = inport_.getData();
    if (geometries.empty()) return;


    const Mesh* geom = dynamic_cast<const Mesh*>(geometries[0]);

    if (geom == NULL) {
        return;
    }

    const Position3dBufferRAM* posBuffer = dynamic_cast<const Position3dBufferRAM*>(geom->getAttributes(0)->getRepresentation<BufferRAM>());

    if (posBuffer == NULL) {
        return;
    }

    const std::vector<vec3>* pos = posBuffer->getDataContainer();
    vec3 maxPos, minPos;

    if (pos->empty()) {
        return;
    }

    maxPos = (*pos)[0];
    minPos = maxPos;
    for (size_t i = 0; i < pos->size(); ++i) {
        maxPos = glm::max(maxPos, (*pos)[i]);
        minPos = glm::min(minPos, (*pos)[i]);
    }

    vec3 centerPos = (geom->getWorldTransform()*geom->getBasisAndOffset()*vec4(0.5f*(maxPos+minPos), 1.f)).xyz();
    vec3 lookFrom = camera_.getLookFrom();
    vec3 dir = centerPos - lookFrom;


    if (glm::length(dir) < glm::epsilon<float>()) {
        dir = vec3(0.f, 0.f, -1.f);
    }

    camera_.setLook(lookFrom, centerPos, camera_.getLookUp());
    return;

    dir = glm::normalize(dir);
    vec3 worldMin = (geom->getWorldTransform()*geom->getBasisAndOffset()*vec4(minPos, 1.f)).xyz();
    vec3 worldMax = (geom->getWorldTransform()*geom->getBasisAndOffset()*vec4(maxPos, 1.f)).xyz();
    vec3 newLookFrom = lookFrom -dir*glm::length(worldMax-worldMin);
    camera_.setLook(newLookFrom, centerPos, camera_.getLookUp());
}

void GeometryRenderProcessorGL::setGlobalShaderParameters(Shader* shader) {
    vec2 dim = static_cast<vec2>(outport_.getDimension());
    shader_->setUniform("screenDim_", dim);
    shader_->setUniform("screenDimRCP_", vec2(1.0f, 1.0f) / dim);
    // camera uniform
    shader->setUniform("viewMatrix_", camera_.viewMatrix());
    shader->setUniform("cameraPosition_", camera_.getLookFrom());
}

void GeometryRenderProcessorGL::updateRenderers() {
    std::vector<const Geometry*> geometries = inport_.getData();

    if (geometries.empty()) {
        while (!renderers_.empty()) {
            delete renderers_.back();
            renderers_.pop_back();
        }
    }

    if (!renderers_.empty()) {
        std::vector<GeometryRenderer*>::iterator it = renderers_.begin();

        while (it!=renderers_.end()) {
            const Geometry* geo = (*it)->getGeometry();
            bool geometryRemoved = true;

            for (size_t j = 0; j < geometries.size(); j++) {
                if (geo == geometries[j]) {
                    geometryRemoved = false;
                    geometries.erase(geometries.begin()+j); //nothing needs to be changed for this geometry
                    break;
                }
            }

            if (geometryRemoved) {
                GeometryRenderer* tmp = (*it);
                it = renderers_.erase(it); //geometry removed, so we delete the old renderer
                delete tmp;
            } else {
                ++it;
            }
        }
    }

    for (size_t i = 0; i < geometries.size() ; ++i) { //create new renderer for new geometries
        GeometryRenderer* renderer = GeometryRendererFactory::getPtr()->create(geometries[i]);

        if (renderer) {
            renderers_.push_back(renderer);
        }
    }
}

void GeometryRenderProcessorGL::resetViewParams() {
    camera_.resetCamera();
}
} // namespace
