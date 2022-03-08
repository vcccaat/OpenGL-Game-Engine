#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int iter;
	std::string sceneName;
	bool saveImg;

public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, std::string sceneName, bool saveImg)
		:ImgGUI(windowWidth, windowHeight) {
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0);
		this->sceneName = sceneName;
		this->saveImg = saveImg;
	}

	void compute_image() {
		iter = iter == 256 ? iter : iter + 1;
		updateImgData(img_data, env, iter, sceneName, saveImg);
	}

	bool mouse_motion_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) {
		if (button == 1) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.orbitCamera(rel.x(), rel.y());
		}
		else if (button == 2) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.zoomCamera(rel.y());
		}
		else if (button == 4) {
			iter = 1;
			img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.f, 0.f, 0.f));
			env.camera.altitudeCamera(rel.y());
		}
		return true;
	}
};

int main() {
	// Path
	std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
	//std::string path = "../resources/scenes/tree.glb";  //bunnyscene

	// Edittable constants
	int height = 500;
	bool saveImg = true;

	// Start application
	int start = path.find_last_of("/");
	int end = path.find_first_of(".glb");
	std::string sceneName = path.substr(start + 1, end - 3);
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(path, (int)height * getAspect(path), height, sceneName, saveImg);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}