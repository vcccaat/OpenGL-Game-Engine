#pragma once
#ifndef CHECK_H
#define CHECK_H

#include <assimp/postprocess.h>    // Post processing flags
#include <assimp/scene.h>          // Output data structure
#include <embree3/rtcore.h>

class Material {
    aiColor3D diffuse;
    float roughness;
    float indexofref;

    Material();
};

class Light {
public:
    std::string name;
    glm::vec3 pos;
    aiColor3D power;
    int sceneindex;
    float width;
    float height;
    float dist;

    enum Type {POINT, AREA, AMBIENT};
    Type type;
    
    Light();
};

class Camera {
public:
    glm::vec3 pos;
    glm::vec3 target; // a vector
    glm::vec3 up;
    float hfov;
    float aspect;
    float phi; //angle around y-axis, as measured from positive x-axis
    float theta; //angle up from x-z plane, clamped to [0:Pi/2]
    float dist;

    Camera(aiCamera* cam);
    Camera();
    glm::vec3 generateRay(float xp, float yp);
    void orbitCamera(float nx, float ny, glm::mat4 trans); // nx ny is the new position of mouse after move
    void zoomCamera(float ny, glm::mat4 trans); // nx ny is the new position of mouse after move
};

struct Environment {
public:
    int width;
    int height;
    Camera camera;
    glm::mat4 camTransMat;
    RTCDevice device;
    RTCScene scene;
    std::vector<Light> lights;

    Environment();
    Environment(std::string objpath, int width, int height);
    void rayTrace(std::vector<glm::vec3>& img_data);
};

Environment startup(int width, int height);
void updateImgData(std::vector<glm::vec3>& img_data, Environment env);

#endif