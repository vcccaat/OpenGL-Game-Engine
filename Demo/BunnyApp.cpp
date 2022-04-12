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


/**************************************** HELPER FUNCTIONS ****************************************/


float getAspect(std::string path) {
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(path, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    if (obj->mNumCameras == 0) return 1.f;
    return obj->mCameras[0]->mAspect;
}

glm::mat4 getTransMatrix(aiNode* rootNode, aiString nodeName) {
    // Dealing with node hierarchy
    aiNode* tempNode = rootNode->FindNode(nodeName);
    glm::mat4 cmt = glm::mat4(1.f);
    glm::mat4 cur;

    // Iterate through all nodes
    while (tempNode != NULL) {
        cur = RTUtil::a2g(tempNode->mTransformation);
        cmt = cur * cmt;
        tempNode = tempNode->mParent;
    }
    return cmt;
}


/**************************************** MATERIAL AND LIGHT CLASS ****************************************/

Material::Material() {
    this->diffuse = glm::vec3(.0, .5, .8);
    this->roughness = .2;
    this->indexofref = 1.5;
}

Light::Light() {}


/**************************************** BUNNYAPP STARTUP METHODS ****************************************/


void BunnyApp::initScene(std::string path, std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight) {
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(path, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    // Mesh parsing
    std::vector<std::vector<glm::vec3>> positions;
    std::vector<std::vector<uint32_t>> indices;
    std::vector<std::vector<glm::vec3>> normals;
    transMatVec = {};
    meshIndToMaterialInd = {};
    traverseNodeHierarchy(positions, indices, normals, obj, obj->mRootNode, transMatVec, glm::mat4(1.f), meshIndToMaterialInd);

    // Mesh inserting
    for (int i = 0; i < positions.size(); ++i) {
        meshes.push_back(std::unique_ptr<GLWrap::Mesh>());
        meshes[i].reset(new GLWrap::Mesh());
        meshes[i]->setAttribute(0, positions[i]);
        meshes[i]->setAttribute(1, normals[i]);
        meshes[i]->setIndices(indices[i], GL_TRIANGLES);
        }

    // Camera initialize
    if (obj->mNumCameras > 0) {
        aiCamera* rawcam = obj->mCameras[0];
        aiNode* rootNode = obj->mRootNode;

        // transform camera
        camTransMat = getTransMatrix(rootNode, rawcam->mName);
        cam->setAspectRatio(rawcam->mAspect);
        cam->setEye(glm::vec3(camTransMat * glm::vec4(rawcam->mPosition.x,rawcam->mPosition.y,rawcam->mPosition.z,1)));
        cam->setFOVY(rawcam->mHorizontalFOV/rawcam->mAspect);
        // Find point closest to origin along target ray using projection in camera space
        glm::vec3 originInCamSpace = glm::vec3(glm::inverse(camTransMat) * glm::vec4(0, 0, 0, 1));
        glm::vec3 originVec = originInCamSpace - glm::vec3(rawcam->mPosition.x, rawcam->mPosition.y, rawcam->mPosition.z);
        glm::vec3 targetVec = glm::vec3(rawcam->mLookAt.x, rawcam->mLookAt.y, rawcam->mLookAt.z) - glm::vec3(rawcam->mPosition.x, rawcam->mPosition.y, rawcam->mPosition.z);
        glm::vec3 projVec = (float) (glm::dot(originVec, targetVec) / pow(glm::length(targetVec), 2)) * targetVec;
        glm::vec3 targetCamSpace = glm::vec3(rawcam->mPosition.x, rawcam->mPosition.y, rawcam->mPosition.z) + projVec;
        glm::vec3 targetGlobal = glm::vec3(camTransMat * glm::vec4(targetCamSpace.x, targetCamSpace.y, targetCamSpace.z, 1));
        cam->setTarget(targetGlobal);
        //cam->setTarget(glm::vec3(0.f, 0.f, 0.f));
        //cam->setAspectRatio(windowWidth / windowHeight);
        // no setUp??
    }

     // Material and light parsing
     materials = parseMats(obj);
     lights = parseLights(obj->mRootNode, obj); // TEMPORARY: only the first pointlight is parsed
}

std::vector<Material> BunnyApp::parseMats(const aiScene* scene) {
    std::vector<Material> mats = {};
    for (int i = 0; i < scene->mNumMaterials; ++i) {
        Material m = Material();
        scene->mMaterials[i]->Get(AI_MATKEY_ROUGHNESS_FACTOR, m.roughness);
        scene->mMaterials[i]->Get(AI_MATKEY_BASE_COLOR, reinterpret_cast<aiColor3D&>(m.diffuse));
        mats.push_back(m);
    }
    return mats;
}

std::vector<Light> BunnyApp::parseLights(aiNode* rootNode, const aiScene* scene) {

    // Initial empty list, then loop over all lights
    std::vector<Light> lights = {};
    for (int i = 0; i < scene->mNumLights; i++) {
        // Construct light
        Light l = Light();
        l.name = scene->mLights[i]->mName.C_Str();
        l.sceneindex = i;

        // Parse area, ambient, and point
        if (RTUtil::parseAreaLight(l.name, l.width, l.height)) {
            // continue; //TEMPORARY
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse; 

            l.type = l.POINT; // testing 2.4 Sum multiple lights
            l.areaNormal = glm::vec3(0, 0, 1);
            l.areaTangent = glm::vec3(0, 1, 0);
            printf("area parsed\n");
        }
        else if (RTUtil::parseAmbientLight(l.name, l.dist)) {
            continue; //TEMPORARY
            l.power = scene->mLights[i]->mColorAmbient;
            l.type = l.AMBIENT;
            printf("amb parsed\n");
        }
        else {
            // continue; //TEMPORARY
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse;
            l.type = l.POINT;
            printf("Point parsed\n"); 
              }

        // transform light
        l.transMat = getTransMatrix(rootNode, scene->mLights[i]->mName);
        if (l.type == l.AREA) {
            //l.areaNormal = glm::normalize(glm::vec3(transMat * glm::vec4(l.areaNormal.x, l.areaNormal.y, l.areaNormal.z, 0)));
        }
        // Push to list
        lights.push_back(l);
    }
    return lights;
}

void BunnyApp::traverseNodeHierarchy(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals, const aiScene* obj, aiNode* cur, std::vector<glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp) {
    if (cur != NULL) {
        transmat = transmat * RTUtil::a2g(cur->mTransformation);
        if (cur->mNumMeshes > 0) {
            
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* temp = obj->mMeshes[cur->mMeshes[i]];
                addMeshToScene(positions, indices, normals, temp, translist, transmat, mp);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy(positions, indices, normals, obj, cur->mChildren[i], translist, transmat, mp);
        }
}
}

void BunnyApp::addMeshToScene(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals, aiMesh* msh, std::vector<glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp){

    // store mesh vertices 
    int curMesh = translist.size();
    positions.push_back({});
    indices.push_back({});
    normals.push_back({});
    for (int i = 0; i < msh->mNumVertices; ++i) {
        glm::vec3 t = reinterpret_cast<glm::vec3&>(msh->mVertices[i]);
        glm::vec3 pos = glm::vec3(glm::vec4(t,1.0)); //transmat[i] * 
        positions[curMesh].push_back(pos);
        // std::cerr << t.x << "," << t.y << "," << t.z << std::endl;

        // access normal of each vertice
        glm::vec3 n = reinterpret_cast<glm::vec3&>(msh->mNormals[i]) ;
        normals[curMesh].push_back(n);
    }
    
    // store mesh indices
    for (int i = 0; i < msh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            indices[curMesh].push_back(reinterpret_cast<uint32_t&>(msh->mFaces[i].mIndices[j]));
        }
    }

    translist.push_back(transmat);
    mp.push_back(msh->mMaterialIndex);
}

BunnyApp::BunnyApp(std::string path, float windowWidth, float windowHeight) : nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false), backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    const std::string resourcePath =
        // PATHEDIT
        cpplocate::locatePath("resources", "", nullptr) + "resources/";
        // cpplocate::locatePath("C:/Users/Ponol/Documents/GitHub/Starter22/resources", "", nullptr) + "C:/Users/Ponol/Documents/GitHub/Starter22/resources/";

    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vert" },
        // { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.geom" },
        //  { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.frag" }
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.frag" }
    }));

    // Set up a simple shader program by passing the shader filenames to the convenience constructor
    fsqProg.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/fsq.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/srgb.frag" }
    }));

    // deferred shading: g-buffer
    gProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/gbuff.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/gbuff.frag" }
        }));
    
    // deferred shading: lighting pass
    lightProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/vlightshade.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/vlightshade.frag" }
        }));

    // shadow pass
    shadowProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/shadow.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/shadow.frag" }
        }));

    // Upload a two-triangle mesh for drawing a full screen quad
    std::vector<glm::vec3> fsqPos = {
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(-1.0f, 1.0f, 0.0f),
    };

    std::vector<glm::vec2> fsqTex = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
    };

    fsqMesh.reset(new GLWrap::Mesh());

    fsqMesh->setAttribute(0, fsqPos);
    fsqMesh->setAttribute(1, fsqTex);

    // Make framebuffer PATHEDIT
    glm::ivec2 myFBOSize = { m_fbsize[0], m_fbsize[1] };
    // glm::ivec2 myFBOSize = { m_fbsize[0] * 1.5, m_fbsize[1] * 1.5};
    fbo.reset(new GLWrap::Framebuffer(myFBOSize));
    gfbo.reset(new GLWrap::Framebuffer(myFBOSize, 3));
    lightfbo.reset(new GLWrap::Framebuffer(myFBOSize));
    shadowfbo.reset(new GLWrap::Framebuffer(myFBOSize,0,true));

    // Default camera, will be overwritten if camera is given in .glb
    cam = std::make_shared<RTUtil::PerspectiveCamera>(
        glm::vec3(6,2,10), // eye
        glm::vec3(-0.2,0.65,0), // target
        glm::vec3(0,1,0), // up
        windowWidth / (float) windowHeight, // aspect
        0.1, 50.0, // near, far
        15.0 * M_PI/180 // fov
    );

    cc.reset(new RTUtil::DefaultCC(cam));

    lightPers = std::make_shared<RTUtil::PerspectiveCamera>(
            glm::vec4(0,0,0,0), // eye
            glm::vec3(0,0,0), // target
            glm::vec3(0,1,0), // up
            1, // aspect
            1.0, 20.0, // near, far
            1 // fov
        );


    initScene(path, cam, windowWidth, windowHeight);
    perform_layout();
    set_visible(true);

    deferred = false;
    toggle = true;
}


/**************************************** UPDATE METHODS FOR BUNNYAPP ****************************************/


bool BunnyApp::keyboard_event(int key, int scancode, int action, int modifiers) {

    if (Screen::keyboard_event(key, scancode, action, modifiers))
       return true;

    // If the user presses the escape key...
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // ...exit the application.
        set_visible(false);
        return true;
    }

    if (key == GLFW_KEY_F && toggle) {
        deferred = !deferred;
        toggle = false;

        if (deferred) {
            std::cout << "deferred shading\n";
        }else {
            std::cout << "forward shading\n";
        }
    } else {
        toggle = true;
    }
    

    return cc->keyboard_event(key, scancode, action, modifiers);
}

bool BunnyApp::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
    return Screen::mouse_button_event(p, button, down, modifiers) ||
           cc->mouse_button_event(p, button, down, modifiers);
}

bool BunnyApp::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
    return Screen::mouse_motion_event(p, rel, button, modifiers) ||
           cc->mouse_motion_event(p, rel, button, modifiers);
}

bool BunnyApp::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
    return Screen::scroll_event(p, rel) ||
           cc->scroll_event(p, rel);
}

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
    GLWrap::checkGLError("deferred shading start");
    //
    // ---------------- geometry pass -------------------
    //
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
    lightfbo->unbind();

    // each light create a different shadow map
    for (int k = 0; k < lights.size(); ++k) { 
        
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
        shadowProg->uniform("mV", lightPers->getViewMatrix());
        shadowProg->uniform("mP", lightPers->getProjectionMatrix());

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
        glClearColor(0, 0, 0, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        
        gfbo->colorTexture(1).bindToTextureUnit(1);
        gfbo->colorTexture(2).bindToTextureUnit(2);
        gfbo->depthTexture().bindToTextureUnit(3);
        shadowfbo->depthTexture().bindToTextureUnit(4);   

        lightProg->use();   
        lightProg->uniform("mVlight", lightPers->getViewMatrix());
        lightProg->uniform("mPlight", lightPers->getProjectionMatrix());
        // std::cout << lightPers->getEye()[0] <<lightPers->getEye()[1] <<lightPers->getEye()[2] << std::endl; 
        lightProg->uniform("mV", cam->getViewMatrix());
        lightProg->uniform("mP", cam->getProjectionMatrix());
        lightProg->uniform("inorm", 1);
        lightProg->uniform("idiff", 2);
        lightProg->uniform("idepth", 3);
        lightProg->uniform("ishadowmap", 4);
        lightProg->uniform("mL", lights[k].transMat);
        lightProg->uniform("lightPos", lights[k].pos);
        lightProg->uniform("power", reinterpret_cast<glm::vec3&>(lights[k].power)); 
        fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
        
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
        // forwardShade(); TEMP!!
        deferredShade();
    }
    else {
        deferredShade();
    }
}