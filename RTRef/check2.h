#pragma once
#ifndef CHECK_H
#define CHECK_H

#include <assimp/postprocess.h>    // Post processing flags
#include <assimp/scene.h>          // Output data structure
#include <embree3/rtcore.h>

class Camera {
public:
    glm::vec3 pos;
    glm::vec3 target; // a vector
    glm::vec3 up;
    float hfov;
    float aspect;
    float angle1; //angle around y-axis, as measured from positive x-axis
    float angle2; //angle up from x-z plane, clamped to [0:Pi/2]

    Camera(aiCamera* cam);
    Camera();
    glm::vec3 generateRay(float xp, float yp);
    void orbitCamera(float nx, float ny); // nx ny is the new position of mouse after move
};

struct Environment {
public:
    int width;
    int height;
    Camera camera;
    glm::mat4 camTransMat;
    RTCDevice device;
    RTCScene scene;

    Environment();
    Environment(std::string objpath, int width, int height);
    void rayTrace(std::vector<glm::vec3>& img_data);
};

Environment startup(int width, int height);
void updateImgData(std::vector<glm::vec3>& img_data, Environment env);

#endif