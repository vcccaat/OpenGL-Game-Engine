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
        scene->mMaterials[i]->Get(AI_MATKEY_REFRACTI, m.indexofref);
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
            continue; //TEMPORARY
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse;
            l.type = l.AREA;
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
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse;
            l.type = l.POINT;
            printf("Point parsed\n");
        }

        // transform light
        glm::mat4 transMat = getTransMatrix(rootNode, scene->mLights[i]->mName);
        l.pos = glm::vec3(transMat * glm::vec4(l.pos.x, l.pos.y, l.pos.z, 1));
        l.transMat = transMat;
        if (l.type == l.AREA) {
            l.areaNormal = glm::normalize(glm::vec3(transMat * glm::vec4(l.areaNormal.x, l.areaNormal.y, l.areaNormal.z, 0)));
        }
        // Push to list
        lights.push_back(l);
        break; //TEMPORARY
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
        positions[curMesh].push_back(t);

        // access normal of each vertice
        glm::vec3 n = reinterpret_cast<glm::vec3&>(msh->mNormals[i]) ;
        normals[curMesh].push_back(n);
    }
    // access normal of each vertice
    // for (int i = 0; i < msh->mNumVertices; ++i) {
    //     glm::vec3 n = reinterpret_cast<glm::vec3&>(msh->mNormals[i]) ;
    //     std::cout << "normal " << n.x << " " << n.y << " " << n.z << std::endl;
    // }
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
        //cpplocate::locatePath("resources", "", nullptr) + "resources/";
        cpplocate::locatePath("C:/Users/Ponol/Documents/GitHub/Starter22/resources", "", nullptr) + "C:/Users/Ponol/Documents/GitHub/Starter22/resources/";

    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vert" },
        // { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.geom" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.frag" }
    }));

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

    initScene(path, cam, windowWidth, windowHeight);
    perform_layout();
    set_visible(true);
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



void BunnyApp::draw_contents() {
    GLWrap::checkGLError("drawContents start");
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // cout << cam->getEye().x << " " << cam->getEye().y << " " << cam->getEye().z  << endl;
    
    glEnable(GL_DEPTH_TEST);
    prog->use();
   
    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());
    //prog->uniform("k_a", glm::vec3(0.1, 0.1, 0.1));
    //prog->uniform("k_d", glm::vec3(0.9, 0.9, 0.9));
    prog->uniform("camPos", cam->getEye()); // to calculate wi
    //prog->uniform("lightDir", glm::normalize(glm::vec3(1.0, 1.0, 1.0)));

    //lights.size() = 1 temporarily as we only parse one light
    for (int k = 0; k < lights.size(); ++k) {
        prog->uniform("lightPos", glm::normalize(lights[k].pos)); // to calculate wo
        for (int i = 0; i < meshes.size(); ++i) {
            Material material = materials[meshIndToMaterialInd[i]];
            // TODO must use frame?
            nori::Microfacet bsdf = nori::Microfacet(material.roughness, material.indexofref, 1.f, material.diffuse);
            prog->uniform("mM", transMatVec[i]);
            prog->uniform("alpha", bsdf.alpha());
            prog->uniform("eta", bsdf.eta());
            prog->uniform("diffuseReflectance", bsdf.diffuseReflectance());
            meshes[i]->drawElements();
        }
    }
    // for OBJ files
    if (lights.size() == 0) {
        //TODO
        prog->uniform("lightDir", glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
        for (int i = 0; i < meshes.size(); ++i) {
            prog->uniform("mM", transMatVec[i]);
            meshes[i]->drawElements();
        }
    }
    
    prog->unuse();
}