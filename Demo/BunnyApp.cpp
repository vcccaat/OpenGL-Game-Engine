#define _USE_MATH_DEFINES
#include <cmath>

#include "BunnyApp.hpp"
#include <nanogui/window.h>
#include <nanogui/layout.h>

#include <cpplocate/cpplocate.h>

// Fixed screen size is awfully convenient, but you can also
// call Screen::setSize to set the size after the Screen base
// class is constructed.
const int BunnyApp::windowWidth = 800;
const int BunnyApp::windowHeight = 600;


// Constructor runs after nanogui is initialized and the OpenGL context is current.
BunnyApp::BunnyApp()
: nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "Bunny Demo", false),
  backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {

    // read bunny obj

    const std::string resourcePath =
        cpplocate::locatePath("resources", "", nullptr) + "resources/";

    // Set up a simple shader program by passing the shader filenames to the convenience constructor
    prog.reset(new GLWrap::Program("program", { 
        { GL_VERTEX_SHADER, resourcePath + "shaders/min.vs" },
        { GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.gs" },
        { GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.fs" }
    }));

    // Create a camera in a default position, respecting the aspect ratio of the window.
    cam = std::make_shared<RTUtil::PerspectiveCamera>(
        glm::vec3(6,2,10), // eye
        glm::vec3(0,0,0), // target
        glm::vec3(0,1,0), // up
        windowWidth / (float) windowHeight, // aspect
        0.1, 50.0, // near, far
        20.0 * M_PI/180 // fov
    );

    cc.reset(new RTUtil::DefaultCC(cam));

    mesh.reset(new GLWrap::Mesh());

    // Mesh was default constructed, but needs to be set up with buffer data
    // These are the vertices of a tetrahedron that fits in the canonical view volume
    std::vector<glm::vec3> positions = {
        glm::vec3(1, 1/sqrt(2), 0),
        glm::vec3(-1, 1/sqrt(2), 0),
        glm::vec3(0, -1/sqrt(2), 1),
        glm::vec3(0, -1/sqrt(2), -1),
    };

    mesh->setAttribute(0, positions);

    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3,
        0, 3, 1,
        1, 3, 2,
    };

    mesh->setIndices(indices, GL_TRIANGLES);

    // NanoGUI boilerplate
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


