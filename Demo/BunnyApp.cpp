#define _USE_MATH_DEFINES
#include <cmath>
#include "BunnyApp.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <cpplocate/cpplocate.h>
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>  
#include <RTUtil/conversions.hpp>

const int BunnyApp::windowWidth = 800;
const int BunnyApp::windowHeight = 600;

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

void BunnyApp::initScene(std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight) {
    // PATHEDIT
    //const string objpath = "../resources/meshes/bunny.obj";
    const string objpath = "../resources/scenes/bunnyscene.glb";
    //const string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.obj";
    // const std::string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(objpath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    // Mesh parsing
    vector<vector<glm::vec3>> positions;
    vector<vector<uint32_t>> indices;
    transMatVec = {};
    traverseNodeHierarchy(positions, indices, obj, obj->mRootNode, transMatVec, glm::mat4(1.f));

    // Mesh inserting
    for (int i = 0; i < positions.size(); ++i) {
        meshes.push_back(std::unique_ptr<GLWrap::Mesh>());
        meshes[i].reset(new GLWrap::Mesh());
        meshes[i]->setAttribute(0, positions[i]);
        meshes[i]->setIndices(indices[i], GL_TRIANGLES);
    }

    // Camera initialize
     if (obj->mNumCameras > 0) {
         aiCamera* rawcam = obj->mCameras[0];
         aiNode* rootNode = obj->mRootNode;
         glm::mat4 camTransMat = getTransMatrix(rootNode, rawcam->mName);

        // transform camera
          glm::vec3 position = glm::vec3(camTransMat * glm::vec4(rawcam->mPosition.x,rawcam->mPosition.y,rawcam->mPosition.z,1));
          glm::vec3 up = glm::vec3(camTransMat * glm::vec4(rawcam->mUp.x,rawcam->mUp.y,rawcam->mUp.z,1));
            up = up + glm::vec3(0,0,.0975);

         cam = make_shared<RTUtil::PerspectiveCamera>(
            position, // eye
            glm::vec3(0, 0.5, 0), // target
            up, // up
            rawcam->mAspect, // aspect
            0.1, 50.0, // near, far
            rawcam->mHorizontalFOV/rawcam->mAspect // fov
            );
     }
}




void BunnyApp::traverseNodeHierarchy(vector<vector<glm::vec3>>& positions, vector<vector<uint32_t>>& indices,const aiScene* obj, aiNode* cur, std::vector<glm::mat4>& translist, glm::mat4 transmat) {
    if (cur != NULL) {
        transmat = transmat * RTUtil::a2g(cur->mTransformation);
        if (cur->mNumMeshes > 0) {
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* temp = obj->mMeshes[cur->mMeshes[i]];
                addMeshToScene(positions, indices, temp, translist, transmat);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy(positions, indices, obj, cur->mChildren[i], translist, transmat);
        }
}
}


void BunnyApp::addMeshToScene(vector<vector<glm::vec3>>& positions, vector<vector<uint32_t>>& indices,aiMesh* msh, std::vector<glm::mat4>& translist, glm::mat4 transmat){

    // store mesh vertices 
    int curMesh = translist.size();
    positions.push_back({});
    indices.push_back({});
    for (int i = 0; i < msh->mNumVertices; ++i) {
        glm::vec3 t = glm::vec3(transmat * glm::vec4(reinterpret_cast<glm::vec3&>(msh->mVertices[i]), 1));
        positions[curMesh].push_back(t);
    }
    // store mesh indices
    for (int i = 0; i < msh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            indices[curMesh].push_back(reinterpret_cast<uint32_t&>(msh->mFaces[i].mIndices[j]));
        }
    }
    translist.push_back(transmat);
}





BunnyApp::BunnyApp() : nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false), backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    const std::string resourcePath =
        // PATHEDIT
        cpplocate::locatePath("resources", "", nullptr) + "resources/";
        // cpplocate::locatePath("C:/Users/Ponol/Documents/GitHub/Starter22/resources", "", nullptr) + "C:/Users/Ponol/Documents/GitHub/Starter22/resources/";

    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vs" },
        { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.gs" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.fs" }
    }));


    // Default camera, will be overwritten if camera is given in .glb
    cam = std::make_shared<RTUtil::PerspectiveCamera>(
        glm::vec3(6,2,10), // eye
        glm::vec3(0,1,0), // target
        glm::vec3(0,1,0), // up
        windowWidth / (float) windowHeight, // aspect
        0.1, 50.0, // near, far
        15.0 * M_PI/180 // fov
    );

    cc.reset(new RTUtil::DefaultCC(cam));

    initScene(cam, windowWidth, windowHeight);
    perform_layout();
    set_visible(true);
}



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

    glEnable(GL_DEPTH_TEST);
    prog->use();
    prog->uniform("mM", glm::mat4(1.0));
    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("k_a", glm::vec3(0.1, 0.1, 0.1));
    prog->uniform("k_d", glm::vec3(0.9, 0.9, 0.9));
    prog->uniform("lightDir", glm::normalize(glm::vec3(1.0, 1.0, 1.0)));

    for (int i = 0; i < meshes.size(); ++i) {
        meshes[i]->drawElements();
    }
    prog->unuse();
}