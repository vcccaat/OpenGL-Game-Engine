#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int iter;
	std::string sceneName;
	bool saveImg;

public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, std::string sceneName, bool saveImg, bool noDefaultCamera)
		:ImgGUI(windowWidth, windowHeight) {
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		this->sceneName = sceneName;
		this->saveImg = saveImg;
		if(noDefaultCamera) env.camera.orbitCamera(0, 0);
	}

	void compute_image() {
		updateImgData(img_data, env, iter, sceneName, saveImg);
		iter = iter == 256 ? iter : iter + 1;
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

int main(int argc, char const* argv[]) {

	if (!(argc > 1)){
		//std::string path = std::string(argv[1]);
		std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/tree.glb";
		//bunnyscene		// bunny
		//tree				// tree
		//staircase			// staircase
		//SkullCycles		// hero image
		//siamese_floor		// alt hero image
		
		// Edittable constants
		int height = 500;
		bool saveImg = false;

		// Start application
		int start = path.find_last_of("/");
		int end = path.find_first_of(".glb");
		std::string sceneName = path.substr(start + 1, end - 3);
		int width;
		float aspect = getAspect(path);
		if (aspect == 0) width = (int)1.3333 * height;
		else width = (int)height * aspect;
		nanogui::init();
		nanogui::ref<BunnyGUI> app = new BunnyGUI(path, width, height, sceneName, saveImg, aspect == 0);
		nanogui::mainloop(16);
		nanogui::shutdown();
	} else {
		std::cout << "Please provide a file path" << std::endl;
	}
	return 0;
}