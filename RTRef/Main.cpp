#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int iter;
	std::string sceneName;
	bool saveImg;
	int maxIter;

public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, std::string sceneName, bool saveImg, bool noDefaultCamera, int maxIter)
		:ImgGUI(windowWidth, windowHeight) {
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		this->sceneName = sceneName;
		this->saveImg = saveImg;
		this->maxIter = maxIter;
		if(noDefaultCamera) env.camera.orbitCamera(0, 0);
	}

	void compute_image() {
		updateImgData(img_data, env, iter, sceneName, saveImg, maxIter);
		iter = iter == maxIter ? iter : iter + 1;
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

	if ((argc > 1)){
		std::string path = std::string(argv[1]);
		//std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
		//bunnyscene		// bunny
		//tree				// tree
		//staircase			// staircase
		//SkullCycles		// hero image
		//siamese_floor		// alt hero image
		
		// Edittable constants
		int height = 500;
		int maxIter = 256;
		bool saveImg = true;

		// Start application
		int start = path.find_last_of("/");
		int end = path.find_last_of(".");
		std::string sceneName = path.substr(start + 1, end - start - 1);
		int width;
		float aspect = getAspect(path);
		if (aspect == 0) width = (int)1.3333 * height;
		else width = (int)height * aspect;
		if (saveImg) std::cout << "Up to " << (int) maxIter / 64 << " images will be saved while the camera stays at a constant orientation" << "\n";
		nanogui::init();
		nanogui::ref<BunnyGUI> app = new BunnyGUI(path, width, height, sceneName, saveImg, aspect == 0, maxIter);
		nanogui::mainloop(16);
		nanogui::shutdown();
	} else {
		std::cout << "Please provide a file path" << std::endl;
	}
	return 0;
}