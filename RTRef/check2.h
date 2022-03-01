#pragma once
#ifndef CHECK_H
#define CHECK_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

struct Environment {
    int width;
    int height;
    Camera camera;
    glm::mat4 camTransMat;
    RTCDevice device;
    RTCScene scene;

    Environment(std::string objpath, int width, int height) {}

    void rayTrace(std::vector<glm::vec3>& img_data) {}
    }
};

Environment startup(int width, int height);
void updateImgData(std::vector<glm::vec3>& img_data, Environment env);

#endif