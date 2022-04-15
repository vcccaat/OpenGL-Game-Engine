#define _USE_MATH_DEFINES
#include <cmath>
#include "BunnyApp.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <cpplocate/cpplocate.h>
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>  
#include <RTUtil/conversions.hpp>
#include "RTUtil/microfacet.hpp"
#include "RTUtil/frame.hpp"
#include <RTUtil\Sky.hpp>



void BunnyApp::forwardShade() {
    GLWrap::checkGLError("drawContents start");

    fbo->bind();
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    prog->use();

    // Plug in camera stuff
    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());

    for (int k = 0; k < lights.size(); ++k) {
        // Plug in lights
        prog->uniform("lightPos"+std::to_string(k+1),  glm::vec3(lights[k].transMat * glm::vec4(lights[k].pos,1.0)));
        prog->uniform("power"+std::to_string(k+1), reinterpret_cast<glm::vec3&>(lights[k].power));      
    }
    for (int i = 0; i < meshes.size(); ++i) {
        // Plug in mesh
        prog->uniform("mM", transMatVec[i]);
        // Plug in materials
        Material material = materials[meshIndToMaterialInd[i]];
        nori::Microfacet bsdf = nori::Microfacet(material.roughness, material.indexofref, 1.f, material.diffuse);
        prog->uniform("alpha", bsdf.alpha());
        prog->uniform("eta", bsdf.eta());
        prog->uniform("diffuseReflectance", bsdf.diffuseReflectance());
        // Draw mesh
        meshes[i]->drawElements();

    }

    prog->unuse();
    fbo->unbind();

    glDisable(GL_DEPTH_TEST);

    fsqProg->use();
    fbo->colorTexture().setParameters(
        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_LINEAR, GL_LINEAR
    );
    fbo->colorTexture().bindToTextureUnit(0);
    fsqProg->uniform("image", 0);
    fsqProg->uniform("exposure", 1.0f);
    // Draw the full screen quad
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    fsqProg->unuse();
    
}



void BunnyApp::deferredShade() {

	
    //
    // ---------------- geometry pass -------------------
    //

    GLWrap::checkGLError("deferred shading start");
    gfbo->bind();
    glViewport(0, 0, m_fbsize[0], m_fbsize[1]);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);

    GLenum attachments[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    gProg->use();
    gProg->uniform("mV", cam->getViewMatrix());
    gProg->uniform("mP", cam->getProjectionMatrix());

    for (int i = 0; i < meshes.size(); ++i) {
        // Plug in mesh
        gProg->uniform("mM", transMatVec[i]);
        // Plug in materials
        Material material = materials[meshIndToMaterialInd[i]];
        nori::Microfacet bsdf = nori::Microfacet(material.roughness, material.indexofref, 1.f, material.diffuse);
        gProg->uniform("alpha", bsdf.alpha());
        gProg->uniform("eta", bsdf.eta());
        gProg->uniform("diffuseReflectance", bsdf.diffuseReflectance());
        // Draw mesh
        meshes[i]->drawElements();
    }

    gProg->unuse();
    glDisable(GL_DEPTH_TEST);  
    gfbo->unbind();

    //
    // ---------------- init accumulation pass -------------------
    //
    lightfbo->bind(); 
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    // lightfbo->unbind();
    
    //
    // ---------------- ambient pass -------------------
    //
    
    // lightfbo->bind();
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ambProg->use();
    ambProg->uniform("mP", cam->getProjectionMatrix());
    ambProg->uniform("mV", cam->getViewMatrix());
    ambProg->uniform("inorm", 1);
    ambProg->uniform("idiff", 2);
    ambProg->uniform("idepth", 3);
    for (int k = 0; k < lights.size(); ++k) {
        if (lights[k].type != lights[k].AMBIENT) continue;
        ambProg->uniform("power", reinterpret_cast<glm::vec3&>(lights[k].power));
        ambProg->uniform("range", lights[k].dist);
    }
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    ambProg->unuse();
    
    //
    // ---------------- sunsky pass -------------------
    //

    sunskyProg->use();

    const float PI = 3.14159265358979323846264;

    RTUtil::Sky sky(85 * PI / 180, 3.0);
    sky.setUniforms(*sunskyProg);

    sunskyProg->uniform("mP", cam->getProjectionMatrix());
    sunskyProg->uniform("mV", cam->getViewMatrix());
    sunskyProg->uniform("inorm", 1);
    sunskyProg->uniform("idiff", 2);
    sunskyProg->uniform("idepth", 3);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

    sunskyProg->unuse();

    // glDisable(GL_BLEND);
    lightfbo->unbind();


    // each light create a different shadow map
    for (int k = 0; k < lights.size(); ++k) {  
        if (lights[k].type == lights[k].AMBIENT) continue;
        
      //
      // ---------------- shadow pass -------------------
      //
		
        shadowfbo->bind(); 
    
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        GLenum attachments[]{ GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1,attachments);

        shadowProg->use(); // TEMP
        
        lightPers->setEye(glm::vec3(lights[k].transMat * glm::vec4(lights[k].pos,1.0)));
        shadowProg->uniform("mVlight", lightPers->getViewMatrix());
        shadowProg->uniform("mPlight", lightPers->getProjectionMatrix());

        // each mesh has a different shadow map
        for (int i = 0; i < meshes.size(); ++i) {          
            shadowProg->uniform("mM", transMatVec[i]);
            meshes[i]->drawElements();
        }
        shadowProg->unuse();
        glDisable(GL_DEPTH_TEST);
        shadowfbo->unbind();

        //
        // ---------------- light shading pass -------------------
        //
		
        lightfbo->bind();     

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        
        gfbo->colorTexture(1).bindToTextureUnit(1);
        gfbo->colorTexture(2).bindToTextureUnit(2);
        gfbo->depthTexture().bindToTextureUnit(3);
        shadowfbo->depthTexture().bindToTextureUnit(4);   

        lightProg->use();   
        lightProg->uniform("mVlight", lightPers->getViewMatrix());
        lightProg->uniform("mPlight", lightPers->getProjectionMatrix());
        lightProg->uniform("mV", cam->getViewMatrix());
        lightProg->uniform("mP", cam->getProjectionMatrix());
        lightProg->uniform("inorm", 1);
        lightProg->uniform("idiff", 2);
        lightProg->uniform("idepth", 3);
        lightProg->uniform("ishadowmap", 4);
        lightProg->uniform("mL", lights[k].transMat);
        lightProg->uniform("lightPos", lights[k].pos);
        lightProg->uniform("power", reinterpret_cast<glm::vec3&>(lights[k].power)); 
        //fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        lightProg->unuse();
        glDisable(GL_BLEND);
        lightfbo->unbind();
    }    
     
    //
    // ---------------- draw to screen  -------------------
    //
	
    fsqProg->use();
    lightfbo->colorTexture().bindToTextureUnit(0);
    fsqProg->uniform("image", 0);
    fsqProg->uniform("exposure", 1.0f);
    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    fsqProg->unuse();
}



void BunnyApp::draw_contents() {
    if (!deferred) {
        forwardShade();
    }
    else {
        deferredShade();
    }
}