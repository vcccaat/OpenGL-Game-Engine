//
//  ImgGUI.cpp
//  Demo
//
//  Created by eschweic on 1/23/19.
//

#include "ImgGUI.hpp"
#include <nanogui/window.h>
#include <glm/glm.hpp>

#include <cpplocate/cpplocate.h>

#include "GLWrap/Util.hpp"
#include "GLWrap/Program.hpp"
#include "GLWrap/Texture2D.hpp"
#include "GLWrap/Mesh.hpp"

namespace RTUtil {

ImgGUI::ImgGUI(int width, int height) :
windowWidth(width),
windowHeight(height),
nanogui::Screen(nanogui::Vector2i(width, height), "NanoGUI Demo", false) {

  // Look up paths to shader source code
  const std::string resourcePath =
    cpplocate::locatePath("resources/shaders", "", nullptr) + "resources/";
  const std::string fsqVertSrcPath = resourcePath + "shaders/fsq.vs";
  const std::string srgbFragSrcPath = resourcePath + "shaders/srgb.fs";

  // Compile shader program
  try {
    srgbShader.reset(new GLWrap::Program("srgb", {
      { GL_VERTEX_SHADER, fsqVertSrcPath }, 
      { GL_FRAGMENT_SHADER, srgbFragSrcPath }
    }));
  } catch (std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    exit(1);
  }

  // Upload a two-triangle mesh for drawing a full screen quad
  std::vector<glm::vec3> positions = {
    glm::vec3(-1.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(-1.0f, 1.0f, 0.0f),
  };

  std::vector<glm::vec2> texCoords = {
    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
  };

  fsqMesh.reset(new GLWrap::Mesh());

  fsqMesh->setAttribute(0, positions);
  fsqMesh->setAttribute(1, texCoords);

  // Allocate texture
  imgTex.reset(new GLWrap::Texture2D(
    glm::ivec2(windowWidth, windowHeight), GL_RGBA32F));

  // Allocate image for display
  img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.0f));

  // Arrange windows in the layout we have described
  perform_layout();
  // Make the window visible and start the application's main loop
  set_visible(true);
}



ImgGUI::~ImgGUI() {
}


bool ImgGUI::keyboard_event(int key, int scancode, int action, int modifiers) {

  // Parent gets first chance to handle event.
  if (Screen::keyboard_event(key, scancode, action, modifiers))
    return true;

  // If the user presses the escape key...
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // ...exit the application.
    set_visible(false);
    return true;
  }

  // Up and down arrows to adjust exposure
  if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
    exposure *= 2.0;
    return true;
  }
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
    exposure /= 2.0;
    return true;
  }

  // Otherwise, the event is not handled here.
  return false;
  //  return cc->keyboard_event(key, scancode, action, modifiers);

}

bool ImgGUI::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
    std::cout << "mouse_button_event" << std::endl;
    return Screen::mouse_button_event(p, button, down, modifiers) ;
}

bool ImgGUI::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
    std::cout << "mouse_motion_event" << std::endl;
    return Screen::mouse_motion_event(p, rel, button, modifiers) ;
}

bool ImgGUI::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
    std::cout << "scroll_event" << std::endl;
    return Screen::scroll_event(p, rel) ;
}


void ImgGUI::draw_contents() {

  // Clear (hardly necessary but makes it easier to recognize viewport issues)
  glClearColor(0.0, 0.2, 1.0, 1.0);
  // nanogui::Color backgroundColor(0.4f, 0.4f, 0.7f, 1.0f);
  // glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
  glClear(GL_COLOR_BUFFER_BIT);

  // Call subclass to update the displayed image
  compute_image();

  // Copy image data to OpenGL
  glBindTexture(GL_TEXTURE_2D, imgTex->id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, img_data.data()); 

  // Set up shader to convert to sRGB and write to default framebuffer
  srgbShader->use();
  imgTex->bindToTextureUnit(0);
  srgbShader->uniform("image", 0);
  srgbShader->uniform("exposure", exposure);

  // Set viewport
  const nanogui::Vector2i framebuffer_size = Screen::framebuffer_size();
  glViewport(0, 0, framebuffer_size.x(), framebuffer_size.y());

  // Draw the full screen quad
  fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

  GLWrap::checkGLError();

}

}
