#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	bool stationary;
	int iter;
public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight):ImgGUI(windowWidth, windowHeight) {
		env = startup(path, windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0);
		iter = 1;
	}
	void compute_image() {
		iter = stationary ? iter + 1 : 1;
		//std::cout << iter << "\n";
		updateImgData(img_data, env, iter); //img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.5f, 0.5f, 0.5f));
	}
	bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers){
		if(button == 1) {
			env.camera.orbitCamera(rel.x(), rel.y());
			stationary = false; // will always move as this function is only called on mouse movement
		} else if (button == 2) {
			env.camera.zoomCamera(rel.y());
			stationary = (rel.y() == 0);
		} else if (button == 4) {
			env.camera.altitudeCamera(rel.y());
			stationary = (rel.y() == 0);
		} else {
			stationary = true;
		}
		return true;
	}
  
};

int main() {
	// Edittable constants
	std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
	//std::string path = "../resources/scenes/bunnyscene.glb";
	int height = 500;

	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(path, (int) height * getAspect(path), height);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}
