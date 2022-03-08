#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int iter;
	std::string sceneName;

public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, std::string sceneName, aiColor3D background)
		:ImgGUI(windowWidth, windowHeight) {
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0);
		env.background = background;
		this->sceneName = sceneName;
	}

	void compute_image() {
		iter = iter == 256? iter : iter + 1;
		updateImgData(img_data, env, iter, sceneName);
	}

	bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers){
		if(button == 1) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.orbitCamera(rel.x(), rel.y());
			stationary = false; // will always move as this function is only called on mouse movement
		} else if (button == 2) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.zoomCamera(rel.y());
			stationary = (rel.y() == 0);
		} else if (button == 4) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.altitudeCamera(rel.y());
		} 
		return true;
	}
};

int main() {
	// Edittable constants
	std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
	//std::string path = "../resources/scenes/tree.glb";  //bunnyscene
	int height = 500;
	std::string sceneName = "bunny";
	aiColor3D background = aiColor3D(.6);
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(path, (int) height * getAspect(path), height, sceneName, background);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}
