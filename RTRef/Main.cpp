#include "check2.h"

#include "../RTUtil/ImgGUI.hpp"

class BunnyGUI : public RTUtil::ImgGUI {
public:
	BunnyGUI(): ImgGUI(800,600) {
		windowWidth = 10;
	}
	void compute_image() {

	}

};

int main() {
	
	nanogui::init();
	nanogui::ref<BunnyGUI> app = new BunnyGUI();
  nanogui::mainloop(16);
	// run();

	nanogui::shutdown();
	return 0;
}
