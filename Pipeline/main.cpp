#include <iostream>
#include "BunnyApp.hpp"

int main(int argc, char const* argv[]) {
    nanogui::init();

    // PATHEDIT
    // const std::string path = "../resources/scenes/tree.glb";
    // const std::string path = "../resources/scenes/bunnyscene2.glb";
    //const std::string path = "../resources/scenes/smoothbunny.glb";
    
    const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene2.glb";
    //const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/smoothbunny.glb";
	
    float aspect = getAspect(path);
    int windowHeight = 500;
    nanogui::ref<BunnyApp> app = new BunnyApp(path, aspect * windowHeight, windowHeight);
    nanogui::mainloop(16);
	
    nanogui::shutdown();
}