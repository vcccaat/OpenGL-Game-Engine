#pragma once
#ifndef CHECK_H
#define CHECK_H

#include <assimp/postprocess.h>    // Post processing flags
#include <assimp/scene.h>          // Output data structure
#include <embree3/rtcore.h>

class Material {
public:
    glm::vec3 diffuse;
    float roughness;
    float indexofref;

    Material();
};

class Light {
public:
    std::string name;
    glm::vec3 pos;
    glm::vec3 areaNormal;
    aiColor3D power;
    int sceneindex;
    float width;
    float height;
    float dist;
    glm::mat4 transMat;

    enum Type { POINT, AREA, AMBIENT };
    Type type;

    Light();
    aiColor3D pointIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hit, glm::vec3 normal, Material material);
    aiColor3D areaIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hit, glm::vec3 normal, Material material);
    aiColor3D ambientIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hit, glm::vec3 normal, Material material);
    // aiColor3D illuminate(glm::vec3 eyeRay, glm::vec3 hit, glm::vec3 normal, Material material);
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
    glm::mat4 transMat;
    float height;

    Camera(aiCamera* cam, glm::vec3 tilt);
    Camera();
    glm::vec3 generateRay(float xp, float yp);
    void transformCamera();
    void untransformCamera();
    void recomputeSpherical();
    void orbitCamera(float nx, float ny); // nx ny is the new position of mouse after move
    void zoomCamera(float ny);
    void altitudeCamera(float ny);
};

struct Environment {
public:
    int width;
    int height;
    Camera camera;
    RTCDevice device;
    RTCScene scene;
    aiColor3D background;
    std::vector<Light> lights;
    std::vector<Material> materials;
    std::vector<int> geomIdToMatInd;

    Environment();
    Environment(std::string objpath, int width, int height);
    void rayTrace(std::vector<glm::vec3>& img_data, float iter);
    aiColor3D castRay(float ox, float oy, float oz, float dx, float dy, float dz);
    aiColor3D shade(glm::vec3 eyeRay, glm::vec3 hitPos, glm::vec3 normal, int geomID);
};

bool isShadowed(RTCScene scene, glm::vec3 lightpos, glm::vec3 hitPos, float maxDist);
float getAspect(std::string path);
Environment startup(std::string path, int width, int height);
void updateImgData(std::vector<glm::vec3>& img_data, Environment env, int iter, std::string sceneName, bool saveImg, int maxIter);
glm::vec3 times(glm::vec3 v, float i);

#endif