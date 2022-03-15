#include <iostream>
#include "DemoApp.hpp"
#include "TetraApp.hpp"
#include "SkyApp.hpp"
#include "BunnyApp.hpp"

int main(int argc, char const* argv[]) {
    nanogui::init();

    if (argc > 1 && std::string(argv[1]) == "Tetra") {
        nanogui::ref<TetraApp> app = new TetraApp();
        nanogui::mainloop(16);
    }
    else if (argc > 1 && std::string(argv[1]) == "Sky") {
        nanogui::ref<SkyApp> app = new SkyApp();
        nanogui::mainloop(16);
    }
    else {
        nanogui::ref<BunnyApp> app = new BunnyApp();
        nanogui::mainloop(16);
    }

    nanogui::shutdown();
}
