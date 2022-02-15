#include <iostream>
#include "DemoApp.hpp"
#include "TetraApp.hpp"
#include "SkyApp.hpp"
#include "../RTRef/minimal.cpp" /////added

int main(int argc, char const *argv[]) {
    main1(); /////added
    return 0; /////added
    /////added
  nanogui::init();
  if (argc > 1 && std::string(argv[1]) == "Tetra") {
    nanogui::ref<TetraApp> app = new TetraApp();
    nanogui::mainloop(16);
  } else if (argc > 1 && std::string(argv[1]) == "Sky") {
    nanogui::ref<SkyApp> app = new SkyApp();
    nanogui::mainloop(16);
  } else {
    nanogui::ref<DemoApp> app = new DemoApp();
    nanogui::mainloop(16);
  }

  nanogui::shutdown();
}
