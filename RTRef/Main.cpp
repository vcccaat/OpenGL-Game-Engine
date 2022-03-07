#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
	Environment env;
	int sample;
	int iter;
public:
	BunnyGUI(std::string path, int windowWidth, int windowHeight, int n):ImgGUI(windowWidth, windowHeight) {
		sample = n;
		iter = 1;
		env = startup(path, windowWidth, windowHeight);
		env.camera.orbitCamera(0, 0);
		// updateImgData(img_data, env);
	}
	void compute_image() {
		iter = iter + 1;
		updateImgData(img_data, env, iter, sample); //img_data = std::vector<glm::vec3>(windowWidth * windowHeight, glm::vec3(0.5f, 0.5f, 0.5f));
	}
	bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers){
		
		if(button == 1) {
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

int main() {
	// Edittable constants
	// std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb";
	std::string path = "../resources/scenes/bunnyscene.glb";
	int height = 500;
	int sample = 256;

	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI(path, (int) height * getAspect(path), height, sample);
	nanogui::mainloop(16);
	nanogui::shutdown();
	return 0;
}
