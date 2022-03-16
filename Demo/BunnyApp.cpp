#define _USE_MATH_DEFINES
#include <cmath>

#include "BunnyApp.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>

#include <cpplocate/cpplocate.h>

/* new import */
#include <assimp/Importer.hpp> 
#include <assimp/postprocess.h>  
using namespace std;


const int BunnyApp::windowWidth = 800;
const int BunnyApp::windowHeight = 600;



void BunnyApp::initScene(std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight) {
    const string objpath = "../resources/scenes/bunnyscene.glb";
    // const string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.obj";
    //const string objpath = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(objpath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    int count = 0;
    traverseNodeHierarchy(obj, obj->mRootNode,count);

     // use camera
    // if (obj->mNumCameras > 0) {
    //     aiCamera* rawcam = obj->mCameras[0];
    //     cam = make_shared<RTUtil::PerspectiveCamera>(
    //         glm::vec3(rawcam->mPosition.x, rawcam->mPosition.y, rawcam->mPosition.z), // eye
    //         glm::vec3(0, 0, 0), // target
    //         glm::vec3(rawcam->mUp.x, rawcam->mUp.y, rawcam->mUp.z), // up
    //         windowWidth / windowHeight, // aspect
    //         0.1, 50.0, // near, far
    //         rawcam->mHorizontalFOV // fov
    //         );
    // }
    
}

void BunnyApp::traverseNodeHierarchy(const aiScene* obj, aiNode* cur, int& count) {
    if (cur != NULL) {
        // transMatrix = transMatrix * RTUtil::a2g(cur->mTransformation);
        if (cur->mNumMeshes > 0) {
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* msh = obj->mMeshes[cur->mMeshes[i]];
                addMeshToScene(msh, count);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy( obj, cur->mChildren[i], count);
        }
}
}


void BunnyApp::addMeshToScene(aiMesh* msh, int& count){
    // aiMesh* mesh = obj->mMeshes[0];
    vector<glm::vec3> positions;
    vector<uint32_t> indices;

    // store mesh vertices 
    for (int i = 0; i < msh->mNumVertices; ++i) {
        positions.push_back(reinterpret_cast<glm::vec3&>(msh->mVertices[i]));
    }
    // store mesh indices
    for (int i = 0; i < msh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            indices.push_back(reinterpret_cast<uint32_t&>(msh->mFaces[i].mIndices[j]));
        }
    }
    if (count == 0){
        mesh->setAttribute(count, positions);
        mesh->setIndices(indices, GL_TRIANGLES);
        count++;
    }
}





BunnyApp::BunnyApp()
: nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false),
  backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    const string resourcePath =
        //cpplocate::locatePath("resources", "", nullptr) + "resources/";
        cpplocate::locatePath("resources", "", nullptr) + "resources/";

    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vs" },
        { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.gs" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.fs" }
    }));


    // Create a camera in a default position, respecting the aspect ratio of the window.
    //  if (obj->mNumCameras == 0) {
        cam = make_shared<RTUtil::PerspectiveCamera>(
            glm::vec3(6,2,10), // eye
            glm::vec3(0,0,0), // target
            glm::vec3(0,1,0), // up
            windowWidth / (float) windowHeight, // aspect
            0.1, 50.0, // near, far
            15.0 * M_PI/180 // fov
        );
    //  }

    cc.reset(new RTUtil::DefaultCC(cam));
    mesh.reset(new GLWrap::Mesh());

    initScene( cam,  windowWidth,  windowHeight);

    
    // mesh->setAttribute(0, positions);
    // mesh->setIndices(indices, GL_TRIANGLES);

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

    mesh->drawElements();
    prog->unuse();
}


