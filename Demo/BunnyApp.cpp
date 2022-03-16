#define _USE_MATH_DEFINES
#include <cmath>

#include "BunnyApp.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>

#include <cpplocate/cpplocate.h>

/* new import */
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
    const string objpath = "../resources/scenes/bunnyscene.glb";
    //const string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.obj";
    // const std::string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(objpath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    vector<vector<glm::vec3>> positions;
    vector<vector<uint32_t>> indices;
    std::vector<glm::mat4> transMatVec = {};
    traverseNodeHierarchy(positions, indices, obj, obj->mRootNode,transMatVec, glm::mat4(1.f));
    //cout << "positions" << positions.size() << endl;
    /*for (int k = 0; k < 2; k++) {
        for (int i = 0; i < 4; i++) {
            printf("%f, %f, %f, %f\n", transMatVec[k][i][0], transMatVec[k][i][1], transMatVec[k][i][2], transMatVec[k][i][3]);
        }
        printf("\n");
    }*/


    m1->setAttribute(0, positions[0]);
    m1->setIndices(indices[0], GL_TRIANGLES);

    m2->setAttribute(0, positions[1]);
    m2->setIndices(indices[1], GL_TRIANGLES);

     // use camera
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
                aiMesh* msh = obj->mMeshes[cur->mMeshes[i]];
                addMeshToScene(positions, indices, msh, translist, transmat);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy(positions, indices, obj, cur->mChildren[i], translist, transmat);
        }
}
}


void BunnyApp::addMeshToScene(vector<vector<glm::vec3>>& positions, vector<vector<uint32_t>>& indices,aiMesh* msh, std::vector<glm::mat4>& translist, glm::mat4 transmat){

    // store mesh vertices 
    int c = translist.size();
    positions.push_back({});
    indices.push_back({});
    std::cout << c;
    for (int i = 0; i < msh->mNumVertices; ++i) {
        glm::vec3 t = glm::vec3(transmat * glm::vec4(reinterpret_cast<glm::vec3&>(msh->mVertices[i]), 1));
        positions[c].push_back(t);
    }
    // store mesh indices
    for (int i = 0; i < msh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            indices[c].push_back(reinterpret_cast<uint32_t&>(msh->mFaces[i].mIndices[j]));
        }
    }
    
    translist.push_back(transmat);
}





BunnyApp::BunnyApp()
: nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false),
  backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    const std::string resourcePath =
        cpplocate::locatePath("resources", "", nullptr) + "resources/";
        // cpplocate::locatePath("C:/Users/Ponol/Documents/GitHub/Starter22/resources", "", nullptr) + "C:/Users/Ponol/Documents/GitHub/Starter22/resources/";

    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vs" },
        { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.gs" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.fs" }
    }));


    // Create a camera in a default position, respecting the aspect ratio of the window.
    // Will be overwritten if another camera is found.
    cam = std::make_shared<RTUtil::PerspectiveCamera>(
        glm::vec3(6,2,10), // eye
        glm::vec3(0,1,0), // target
        glm::vec3(0,1,0), // up
        windowWidth / (float) windowHeight, // aspect
        0.1, 50.0, // near, far
        15.0 * M_PI/180 // fov
    );

    cc.reset(new RTUtil::DefaultCC(cam));
    m1.reset(new GLWrap::Mesh());
    m2.reset(new GLWrap::Mesh());

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

    m1->drawElements();
    m2->drawElements();
    prog->unuse();
}


