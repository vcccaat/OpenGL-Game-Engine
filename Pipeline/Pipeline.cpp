#define _USE_MATH_DEFINES
#include <cmath>
#include "Pipeline.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <cpplocate/cpplocate.h>
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>  
#include <RTUtil/conversions.hpp>
#include "RTUtil/microfacet.hpp"
#include "RTUtil/frame.hpp"
#include "Helper.hpp"

/**************************************** HELPER FUNCTIONS ****************************************/

float getAspect(std::string path) {
    Assimp::Importer importer;
    // const aiScene* obj = importer.ReadFile(path, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    const aiScene* obj = importer.ReadFile(path,
    aiProcess_LimitBoneWeights |
    aiProcess_Triangulate |
    aiProcess_SortByPType);
    if (obj->mNumCameras == 0) return 1.f;
    return obj->mCameras[0]->mAspect;
}

double getSecondsSinceEpoch() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / ((double) 1000);
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

Material::Material(glm::vec3 diffuse) {
    this->diffuse = diffuse;
    this->roughness = .2;
    this->indexofref = 1.5;
}

Light::Light() {}



/**************************************** Pipeline STARTUP METHODS ****************************************/


void Pipeline::initScene(std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight) {
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(GlobalPath, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    transMatVec = {};
    meshIndToMaterialInd = {};

    // add mesh to scene and store meshes' model matrix
    traverseNodeHierarchy( obj, obj->mRootNode, glm::mat4(1.f));

    // number of positions equal to number of meshes in the scene
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
    }


     // Add default light for animation
     if (obj->mNumLights > 0){
         lights = parseLights(obj->mRootNode, obj);
     }
     else {
        Light defaultLight = Light();
        defaultLight.pos = glm::vec3(2,5,0);
        defaultLight.type = defaultLight.POINT;
        defaultLight.power = aiColor3D(300);
        defaultLight.transMat = glm::mat4(1.f);
        lights.push_back(defaultLight);
     }

    // Add default material for animation
    if (obj->mNumMaterials > 0){
        materials = parseMaterials(obj);
    }
    else{
        Material m1 = Material(glm::vec3(0.2,0.31,0.46));
        Material m2 = Material(glm::vec3(0.46,0.2,0.4));
        materials.push_back(m1);
        materials.push_back(m2);
    }


	// Animation
    if (obj->mNumAnimations > 0){
        animationOfName = {};
        for (int i = 0; i < obj->mAnimations[0]->mNumChannels; ++i) {
            NodeAnimate na = NodeAnimate();
            na.keyframePos = {};
            na.keyframeRot = {};
            na.keyframeScale = {};
            aiNodeAnim* curChannel = obj->mAnimations[0]->mChannels[i];
            std::string nodeName = curChannel->mNodeName.C_Str();
            na.name = nodeName;
    
            for (int j = 0; j < curChannel->mNumPositionKeys; ++j) {
                KeyframePos k;
                k.time = (float) curChannel->mPositionKeys[j].mTime / (float) obj->mAnimations[0]->mTicksPerSecond;
                k.pos = RTUtil::a2g(curChannel->mPositionKeys[j].mValue);
                std::cout << nodeName<< "translation \n";
                printf("%f: %f %f %f\n",k.time, k.pos.x,k.pos.y,k.pos.z);
                na.keyframePos.push_back(k);
            }
            for (int j = 0; j < curChannel->mNumRotationKeys; ++j) {
                KeyframeRot k;
                k.time = (float) curChannel->mRotationKeys[j].mTime / (float) obj->mAnimations[0]->mTicksPerSecond;
                k.rot = curChannel->mRotationKeys[j].mValue;
                std::cout << nodeName<< "rotation \n";
                printf("%f : %f, %f, %f, %f\n" ,k.time, k.rot.w,  k.rot.x , k.rot.y , k.rot.z) ;
                na.keyframeRot.push_back(k);
            }        
            for (int j = 0; j < curChannel->mNumScalingKeys; ++j) {
                KeyframeScale k;
                k.time = (float) curChannel->mScalingKeys[j].mTime / (float) obj->mAnimations[0]->mTicksPerSecond;
                k.scale = RTUtil::a2g(curChannel->mScalingKeys[j].mValue);
                na.keyframeScale.push_back(k);
            }
            animationOfName.insert({ nodeName, na });
        }

        // Animation metadata
        startTime = getSecondsSinceEpoch();
        curTime = startTime;
        int ticksPerSec = obj->mAnimations[0]->mTicksPerSecond;
        int totalTicks = obj->mAnimations[0]->mDuration;
        totalTime = (double) totalTicks / (double) ticksPerSec;
    }

}

std::vector<Material> Pipeline::parseMaterials(const aiScene* scene) {
    std::vector<Material> mats = {};
    for (int i = 0; i < scene->mNumMaterials; ++i) {
        Material m = Material();
        scene->mMaterials[i]->Get(AI_MATKEY_ROUGHNESS_FACTOR, m.roughness);
        scene->mMaterials[i]->Get(AI_MATKEY_BASE_COLOR, reinterpret_cast<aiColor3D&>(m.diffuse));
        mats.push_back(m);
    }
    return mats;
}

std::vector<Light> Pipeline::parseLights(aiNode* rootNode, const aiScene* scene) {

    // Initial empty list, then loop over all lights
    std::vector<Light> lights = {};
    for (int i = 0; i < scene->mNumLights; i++) {
        // Construct light
        Light l = Light();
        l.name = scene->mLights[i]->mName.C_Str();
        l.sceneindex = i;

        // Parse area, ambient, and point
        if (RTUtil::parseAreaLight(l.name, l.width, l.height)) {
            continue; //TEMPORARY
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse; 

            l.type = l.AREA; // testing 2.4 Sum multiple lights
            l.areaNormal = glm::vec3(0, 0, 1);
            l.areaTangent = glm::vec3(0, 1, 0);
            printf("area parsed\n");
        }
        else if (RTUtil::parseAmbientLight(l.name, l.dist)) {
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

void Pipeline::traverseNodeHierarchy( const aiScene* obj, aiNode* cur, glm::mat4 transmat) {
    if (cur != NULL) {
        transmat = transmat * RTUtil::a2g(cur->mTransformation);
        if (cur->mNumMeshes > 0) {
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* mesh = obj->mMeshes[cur->mMeshes[i]];
                idToName.push_back(mesh->mName.C_Str());
                addMeshToScene( mesh, transmat);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy( obj, cur->mChildren[i], transmat);
        }
}
}

void Pipeline::addMeshToScene( aiMesh* msh,  glm::mat4 transmat){

    // store mesh vertices 
    int curMesh = transMatVec.size();
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
    transMatVec.insert({ msh->mName.C_Str(), transmat });
    // transMatVec.push_back(transmat);
    meshIndToMaterialInd.push_back(msh->mMaterialIndex);
}

Pipeline::Pipeline(std::string path, float windowWidth, float windowHeight) : nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false), backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    const std::string resourcePath =
        // PATHEDIT
        cpplocate::locatePath("resources", "", nullptr) + "resources/";
        // cpplocate::locatePath("C:/Users/Ponol/Documents/GitHub/Starter22/resources", "", nullptr) + "C:/Users/Ponol/Documents/GitHub/Starter22/resources/";

    // forward shading
    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vert" },
        // { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.geom" },
        //  { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.frag" }
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.frag" }
    }));

    // deferred shading: g-buffer
    gProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/gbuff.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/gbuff.frag" }
        }));
    
    // sunsky pass
    sunskyProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/ambient.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/sunsky.frag" }
        }));

    // ambient pass
    ambProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/ambient.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/ambient.frag" }
        }));

    // lighting pass
    lightProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/vlightshade.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/vlightshade.frag" }
        }));

    // shadow pass
    shadowProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/shadow.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/shadow.frag" }
        }));

    // accumulation pass
    accProg.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/fsq.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/blur.frag" }
    }));

    // srgb
    fsqProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/fsq.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/srgb.frag" }
        }));

    // merge pass
    mergeProg.reset(new GLWrap::Program("program", {
        { GL_VERTEX_SHADER, resourcePath + "shaders/fsq.vert" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/merge.frag" }
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
    std::vector<std::pair<GLenum, GLenum>> floatFormat;
    for (int i =0; i< 5; ++i){
        floatFormat.push_back(std::make_pair(GL_RGBA32F, GL_RGBA));
    }
    
    fbo.reset(new GLWrap::Framebuffer(myFBOSize));
    gfbo.reset(new GLWrap::Framebuffer(myFBOSize, 3));
    lightfbo.reset(new GLWrap::Framebuffer(myFBOSize));
    shadowfbo.reset(new GLWrap::Framebuffer(myFBOSize,0,true));
    blurHorfbo.reset(new GLWrap::Framebuffer(myFBOSize));
    blurVerfbo.reset(new GLWrap::Framebuffer(myFBOSize, 5));
    mergefbo.reset(new GLWrap::Framebuffer(myFBOSize));

    // Default camera, will be overwritten if camera is given in .glb
    cam = std::make_shared<RTUtil::PerspectiveCamera>(
        glm::vec3(6,6,10), // eye  6,2,10
        glm::vec3(-0.2,0.65,0), // target
        glm::vec3(0,1,0), // up
        windowWidth / (float) windowHeight, // aspect
        0.1, 50.0, // near, far
        25.0 * M_PI/180 // fov  15.0 * M_PI/180
    );

    cc.reset(new RTUtil::DefaultCC(cam));

    lightPers = std::make_shared<RTUtil::PerspectiveCamera>(
            glm::vec4(0,0,0,0), // eye
            glm::vec3(0,0,0), // target
            glm::vec3(0,1,0), // up
            1, // aspect
            1.0, 40.0, // near, far
            1 // fov
        );

    GlobalPath = path;
    initScene( cam, windowWidth, windowHeight);
    perform_layout();
    set_visible(true);

    deferred = true;
    toggle = true;
}



void Pipeline::draw_contents() {
    // Update current time
    curTime = getSecondsSinceEpoch();
    float t = std::fmod(curTime - startTime, totalTime);
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(GlobalPath, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    // if the scene has animation, traverse the tree to update TRS
    if (animationOfName.size() > 0){
        traverseTree(obj, obj->mRootNode, glm::mat4(1.f), t);
    }
    


    forwardShade();
    // if (!deferred) {
    //     _CrtDumpMemoryLeaks();
    //     deferred = true;
    // }
    return;
    //if (!deferred) {
    //    forwardShade();
    //}
    //else {
    //    deferredShade();  //deferredShade   // TEMP: only use forward shading for animation
    //}
}

