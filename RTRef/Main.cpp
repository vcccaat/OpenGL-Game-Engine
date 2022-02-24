#include "check2.h"

#include "../RTUtil/ImgGUI.hpp"

class BunnyGUI : public RTUtil::ImgGUI {
	void compute_image() {

	}
};

int main() {
	BunnyGUI bgui();
	nanogui::init();
	run();
	return 0;
}
