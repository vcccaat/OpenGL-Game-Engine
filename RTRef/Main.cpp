#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
public:
	BunnyGUI(): ImgGUI(800, 600) {
		// should init camera, scene here
		updateImgData(img_data,windowWidth,windowHeight);
	}
	void compute_image() {
		// always updating 
		// img_data = std::vector<glm::vec3>(windowWidth * windowHeight,glm::vec3(0.5f,0.5f,0.5f));
	}
	// bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers){
		// cam.orbitCamera(-rel.x(), -rel.y()); // check num is positive
	// }
  
};

int main() {
	
	bool window = true;
	if (window) {
		nanogui::init();
		nanogui::ref<BunnyGUI> app = new BunnyGUI();
		nanogui::mainloop(16);
		nanogui::shutdown();
	} else {
		run();
	}
	return 0;
}
