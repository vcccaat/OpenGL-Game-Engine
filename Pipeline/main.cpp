#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

#include <iostream>
#include "Pipeline.hpp"


int main(int argc, char const* argv[]) {
    nanogui::init();

    // PATHEDIT
    const std::string path = "../resources/scenes/CesiumMan.glb";  //BoxAnimated.glb, CesiumMan.glb, RiggedFigure.glb, mosh_cmu_*.glb
    // const std::string path = "../resources/scenes/tree.glb";
    //const std::string path = "../resources/scenes/bunnyscene2.glb";
    //const std::string path = "../resources/scenes/smoothbunny.glb";
    
    // const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene2.glb";
    //const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/smoothbunny.glb";
    // const std::string path = "C:/Users/Ponol/Documents/GitHub/Starter22/resources//scenes/CesiumMan.glb";
	
    float aspect = getAspect(path);
    int windowHeight = 500;
    nanogui::ref<Pipeline> app = new Pipeline(path, aspect * windowHeight, windowHeight);
    nanogui::mainloop(16);
	
    nanogui::shutdown();
}