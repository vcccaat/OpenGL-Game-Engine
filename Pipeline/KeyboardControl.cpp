#include "Pipeline.hpp"

bool Pipeline::keyboard_event(int key, int scancode, int action, int modifiers) {

    if (Screen::keyboard_event(key, scancode, action, modifiers))
       return true;

    // If the user presses the escape key...
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // ...exit the application.
        set_visible(false);
        return true;
    }

    // if (key == GLFW_KEY_F && toggle) {
    //     deferred = !deferred;
    //     toggle = false;

    //     if (deferred) {
    //         std::cout << "deferred shading\n";
    //     }else {
    //         std::cout << "forward shading\n";
    //     }
    // } else {
    //     toggle = true;
    // }


    // player input system
    glm::vec3 viewDir = cam->getEye() - cam->getTarget();
    if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
        float camSpeed = 0.2f;
        // if camera is rotated, shifting in z-axis in camera space is not the same as in world space
        glm::vec3 v = glm::vec3(glm::inverse(cam->getViewMatrix()) * glm::vec4(0,0,1,0));
        cam->setEye(cam->getEye() - v * camSpeed);
    }
    if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
        float camSpeed = 0.2f;
        glm::vec3 v = glm::vec3(glm::inverse(cam->getViewMatrix()) * glm::vec4(0,0,1,0));
        cam->setEye(cam->getEye() + v * camSpeed);
    }
    if (key == GLFW_KEY_A && action != GLFW_RELEASE) {  
        float camSpeed = 0.08f;
        glm::vec3 v = glm::vec3(glm::inverse(cam->getViewMatrix()) * glm::vec4(1,0,0,0));
        // always keep the same eye direction for camera
        cam->setEye(cam->getEye() - v * camSpeed);
        // therefore need to update target pos
        cam->setTarget(cam->getEye()- viewDir );
    }
    if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
        float camSpeed = 0.08f;
        glm::vec3 v = glm::vec3(glm::inverse(cam->getViewMatrix()) * glm::vec4(1,0,0,0));
        cam->setEye(cam->getEye() + v * camSpeed);
        cam->setTarget(cam->getEye()-viewDir);
    }
    

    return cc->keyboard_event(key, scancode, action, modifiers);
}

bool Pipeline::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
    return Screen::mouse_button_event(p, button, down, modifiers) ||
           cc->mouse_button_event(p, button, down, modifiers);
}

bool Pipeline::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
    return Screen::mouse_motion_event(p, rel, button, modifiers) ||
           cc->mouse_motion_event(p, rel, button, modifiers);
}

bool Pipeline::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
    return Screen::scroll_event(p, rel) ||
           cc->scroll_event(p, rel);
}