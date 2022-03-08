#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int iter;
	std::string sceneName;
	bool writeImg;

public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, std::string sceneName)
		:ImgGUI(windowWidth, windowHeight) {
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0);
		this->sceneName = sceneName;
		this->writeImg = writeImg;
	}

	void compute_image() {
		iter = iter == 256 ? iter : iter + 1;
		updateImgData(img_data, env, iter, sceneName, writeImg);
	}

	bool mouse_motion_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) {
		if (button == 1) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.orbitCamera(rel.x(), rel.y());
		} else if (button == 2) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.zoomCamera(rel.y());
		} else if (button == 4) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.altitudeCamera(rel.y());
		}
		return true;
	}
};

int main(int argc, char const* argv[]) {
	std::string path;
	if (argc > 1) {
		path = std::string(argv[1]);
	} else {
		path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
		//std::string path = "../resources/scenes/bunnyscene.glb";
	}
	
	// Edittable constants
	bool writeImg = false;
	int height = 500;
	std::string sceneName = "bunny";
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(path, (int) height * getAspect(path), height, sceneName);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}