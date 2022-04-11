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
        // PATHEDIT
        //const string path = "../resources/meshes/bunny.obj";
        const std::string path = "../resources/scenes/bunnyscene2.glb";
        //const string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.obj";
        // const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/smoothbunny.glb";
        float aspect = getAspect(path);
        int windowHeight = 500;
        nanogui::ref<BunnyApp> app = new BunnyApp(path, aspect * windowHeight, windowHeight);
        nanogui::mainloop(16);
    }
    nanogui::shutdown();
}
