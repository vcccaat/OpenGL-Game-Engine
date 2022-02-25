#include <iostream>
#include "../RTUtil/ImgGUI.hpp"
#include <RTRef/check2.h>

class BunnyGUI : public RTUtil::ImgGUI {
public:
	BunnyGUI(): ImgGUI(400,400) { //C:/Users/Ponol/Documents/GitHub/Starter22/resources/shaders
		std::cout << windowWidth << windowHeight << "\n";
		img_data = getImgData(windowWidth,windowHeight);
	}
	void compute_image() {
		// always updating 
		// img_data = std::vector<glm::vec3>(windowWidth * windowHeight,glm::vec3(0.5f,0.5f,0.5f));
	}
};

int main() {
	bool window = false;
	if (window) {
		nanogui::init();
		nanogui::ref<BunnyGUI> app = new BunnyGUI();
		nanogui::mainloop(16);
		nanogui::shutdown();
	} else {
		run();
	}
	// getImgData(800,800);  //not work for rectangle screen, check!!
	return 0;
}
