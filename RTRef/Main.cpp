#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
public:
	BunnyGUI(int windowWidth, int windowHeight):ImgGUI(windowWidth, windowHeight) {
		env = startup(windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0, env.camTransMat);
	}
	void compute_image() {
		updateImgData(img_data, env); //img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.5f, 0.5f, 0.5f));
	}
	bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers){
		if(button == 1) {
			env.camera.orbitCamera(rel.x(), rel.y(), env.camTransMat); // check num is positive
		} else if (button == 2) {
			env.camera.zoomCamera(rel.y(), env.camTransMat); // check num is positive
		}
		return true;
	}
  
};

int main() {
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(800, 100);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}
