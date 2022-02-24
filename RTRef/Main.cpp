#include "check2.h"
#include <iostream>
#include "../RTUtil/ImgGUI.hpp"

class BunnyGUI : public RTUtil::ImgGUI {
public:
	BunnyGUI(): ImgGUI(800,800) {
		std::cout << windowWidth << windowHeight << "\n";
		
		img_data = getImgData(windowWidth,windowHeight);
		
	}
	void compute_image() {
		// always updating 
		// img_data = std::vector<glm::vec3>(windowWidth * windowHeight,glm::vec3(255,255,255));
}

};



int main() {
	
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI();
  nanogui::mainloop(16);
	nanogui::shutdown();
	// run();
	// getImgData(800,800);  //not work for rectangle screen, check!!


	
	return 0;
}
