// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <stdio.h>
#include <../RTUtil/conversions.hpp>
#include <../RTUtil/output.hpp>
#include <assimp/Importer.hpp>    // C++ importer interface
#include <limits>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <cpplocate/cpplocate.h>
#include <stb_image_write.h>
#include <GLWrap/Program.hpp>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "check2.h"

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#endif
#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif


/**************************************** CAMERA ****************************************/


Camera::Camera(aiCamera *cam) {
    this->pos = glm::vec3(cam->mPosition.x, cam->mPosition.y, cam->mPosition.z);
    this->target = glm::vec3(cam->mLookAt.x, cam->mLookAt.y, cam->mLookAt.z);
    this->up = glm::vec3(cam->mUp.x, cam->mUp.y, cam->mUp.z);
    this->hfov = cam->mHorizontalFOV;
    this->aspect = cam->mAspect;
    this->phi = 1.3;
    this->theta = 1.8;
    this->dist = 8;
    //std::cout << "camera position: " << this->pos << ", camera target: " << this->target << ", camera up: " << this->up << this->hfov << this->aspect << std::endl;
}

Camera::Camera() {
    this->pos = glm::vec3(3.f, 4.f, 5.f);
    this->target = glm::vec3(-3.f, -4.f, -5.f);
    this->up = glm::vec3(0.f, 1.f, 0.f);
    this->hfov = 0.5;
    this->aspect = 1;
    this->phi = 1.3;
    this->theta = 1.8;
    this->dist = 8;
}

glm::vec3 Camera::generateRay(float xp, float yp) {
    glm::vec3 w = glm::normalize(-this->target);
    glm::vec3 u = glm::normalize(glm::cross(this->up, w));
    glm::vec3 v = glm::normalize(glm::cross(w, u));

    double wide = 2 * tan(hfov / 2);
    double h = wide / this->aspect;
    double u_small = xp * wide;
    double v_small = yp * h;
    u_small -= wide / 2;
    v_small -= h / 2;
    float x = u.x * u_small + v.x * v_small - w.x;
    float y = u.y * u_small + v.y * v_small - w.y;
    float z = u.z * u_small + v.z * v_small - w.z;
    return glm::vec3(x, y, z);
}

void transformCamera(Camera& cam, glm::mat4 cmt) {
    cam.pos = glm::vec3(cmt * glm::vec4(cam.pos.x, cam.pos.y, cam.pos.z, 1));
    cam.target = glm::vec3(cmt * glm::vec4(cam.target.x, cam.target.y, cam.target.z, 0));
    cam.up = glm::vec3(cmt * glm::vec4(cam.up.x, cam.up.y, cam.up.z, 0));
}

void untransformCamera(Camera& cam, glm::mat4 cmt) {
    cam.pos = glm::vec3(glm::inverse(cmt) * glm::vec4(cam.pos.x, cam.pos.y, cam.pos.z, 1));
    cam.target = glm::vec3(glm::inverse(cmt) * glm::vec4(cam.target.x, cam.target.y, cam.target.z, 0));
    cam.up = glm::vec3(glm::inverse(cmt) * glm::vec4(cam.up.x, cam.up.y, cam.up.z, 0));
}

/// nx ny is the new position of mouse after move
void Camera::orbitCamera(float nx, float ny, glm::mat4 trans){

    // untransformCamera(*this, trans);
    // "Sensitivity" of mouse movement
    float pi = 3.1415926;
    float scale = .0075;

    theta -= nx * scale;
    phi += ny * scale;
    if (phi < 0) phi = 0;
    else if (phi > pi) phi = pi;

    //std::cout << theta << "\n" << phi << "\n\n";

    this->pos.x = dist * sin(phi) * cos(theta);
    this->pos.y = dist * cos(phi);
    this->pos.z = dist * sin(phi) * sin(theta);
    this->target = -this->pos;
    
    // transformCamera(*this, trans);
}


/**************************************** GIVEN FUNCTIONS ****************************************/


void errorFunction(void *userPtr, enum RTCError error, const char *str) {
    printf("error %d: %s\n", error, str);
}

RTCDevice initializeDevice() {
    RTCDevice device = rtcNewDevice(NULL);
    if (!device)
        printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));
    rtcSetDeviceErrorFunction(device, errorFunction, NULL);
    return device;
}

void waitForKeyPressedUnderWindows() {
#if defined(_WIN32)
    HANDLE hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hStdOutput, &csbi)) {
        printf("GetConsoleScreenBufferInfo failed: %d\n", GetLastError());
        return;
    }
    /* do not pause when running on a shell */
    if (csbi.dwCursorPosition.X != 0 || csbi.dwCursorPosition.Y != 0)
        return;
    /* only pause if running in separate console window. */
    printf("\n\tPress any key to exit...\n");
    int ch = getch();
#endif
}


/**************************************** SCENE, CAMERA, AND RAY HELPERS ****************************************/


void addMeshToScene(RTCDevice device, RTCScene scene, aiMesh *mesh, glm::mat4 transMatrix) {
    // get vertices from aiscene meshList by mMeshes indexes
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    float *vertices = (float *)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                                                       3 * sizeof(float), 3 * mesh->mNumVertices);
    unsigned *indices = (unsigned *)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                                                       3 * sizeof(unsigned), 3 * mesh->mNumFaces);
    for (int i = 0; i < mesh->mNumVertices; ++i) {
        float x = mesh->mVertices[i][0];
        float y = mesh->mVertices[i][1];
        float z = mesh->mVertices[i][2];
        glm::vec4 chg = transMatrix * glm::vec4(x, y, z, 1) ;
        vertices[3 * i + 0] = chg.x;
        vertices[3 * i + 1] = chg.y;
        vertices[3 * i + 2] = chg.z;
    }
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        indices[3 * i + 0] = mesh->mFaces[i].mIndices[0];
        indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
        indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
    }
    rtcCommitGeometry(geom);
    rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
}

void traverseNodeHierarchy(RTCDevice device, RTCScene scene, const aiScene *aiscene, aiNode *cur, glm::mat4 transMatrix) {
    // top down, compute transformation matrix while traversing down the tree
    if (cur != NULL) {
        transMatrix = RTUtil::a2g(cur->mTransformation)*transMatrix;
        // when it reaches mesh, transform the vertices
        if (cur->mNumMeshes > 0) {
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* mesh = aiscene->mMeshes[cur->mMeshes[i]];
                addMeshToScene(device, scene, mesh, transMatrix);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy(device, scene, aiscene, cur->mChildren[i], transMatrix);
        }
    }
}

RTCScene initializeScene(RTCDevice device, const aiScene *aiscene, Camera &cam) {
    RTCScene scene = rtcNewScene(device);
    traverseNodeHierarchy(device, scene, aiscene, aiscene->mRootNode, glm::mat4(1.f));
    rtcCommitScene(scene);
    return scene;
}

glm::mat4 getCameraMatrix(const aiScene* obj) {
    // Dealing with node hierarchy
    aiNode* rootNode = obj->mRootNode;
    aiCamera* camera = obj->mCameras[0]; //get the first camera
    aiNode* tempNode = rootNode->FindNode(camera->mName);
    glm::mat4 cmt = glm::mat4(1.f);
    glm::mat4 cur;

    // Iterate through all nodes
    while (tempNode != NULL) {
        cur = RTUtil::a2g(tempNode->mTransformation);
        cmt = cur * cmt;
        tempNode = tempNode->mParent;
    }
    return cmt;
}



aiColor3D castRay(RTCScene scene, float ox, float oy, float oz, float dx, float dy, float dz) {
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    struct RTCRayHit rayhit;
    rayhit.ray.org_x = ox;
    rayhit.ray.org_y = oy;
    rayhit.ray.org_z = oz;
    rayhit.ray.dir_x = dx;
    rayhit.ray.dir_y = dy;
    rayhit.ray.dir_z = dz;
    rayhit.ray.tnear = 0;
    rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(scene, &context, &rayhit);

    //printf("%f, %f, %f: ", ox, oy, oz);`
    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        glm::vec3 col = glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
        float out = glm::dot(glm::normalize(col), glm::normalize(glm::vec3(1.f, 1.f, 1.f)));
        out = out < 0 ? 0 : out;
        return aiColor3D(out);
    }
    return aiColor3D();
}


aiLight getLight(const aiScene* scene){
    float width;
    float height;
    float range;
    aiColor3D power;
    if (scene->HasLights()){
        aiLight** lights = scene->mLights;
        for (int i = 0; i < scene->mNumLights; i++){
            // get point light
            char* lightName = scene->mLights[i]->mName.data;
            printf("%s\n",lightName);
            if (RTUtil::parseAreaLight(lightName,width,height)){
                power =  scene->mLights[i]->mColorDiffuse;
                std::cout << "area" << power.r << power.g << power.b << std::endl;
            }
            else if (RTUtil::parseAmbientLight(lightName,range)){
                // here power is radiance
                power =  scene->mLights[i]->mColorDiffuse;
                std::cout << "ambient" << power.r << power.g << power.b << std::endl;
            } else {
                power =  scene->mLights[i]->mColorDiffuse;
                std::cout << "point light" << power.r << power.g << power.b << std::endl;
            }
        }
    }
}


/**************************************** ENVIRONMENT ****************************************/


Environment::Environment() {}

Environment::Environment(std::string objpath, int width, int height) {
    this->width = width;
    this->height = height;

    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(objpath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    this->device = initializeDevice();

    this->camera = Camera(obj->mCameras[0]); 
    this->camTransMat = getCameraMatrix(obj);
    transformCamera(this->camera, camTransMat);

    getLight(obj);

    this->scene = initializeScene(this->device, obj, this->camera);
}

void Environment::rayTrace(std::vector<glm::vec3>& img_data) {
    glm::vec3 dir;
    for (int j = 0; j < height; ++j) for (int i = 0; i < width; ++i) {
        dir = camera.generateRay((i + .5) / height, (j + .5) / width);
        aiColor3D col = castRay(scene, camera.pos.x, camera.pos.y, camera.pos.z, dir.x, dir.y, dir.z);
        img_data[j * width + i] = glm::vec3(col.r, col.g, col.b);
    }
}


/**************************************** MAIN FUNCTIONS ****************************************/


Environment startup(int width, int height) {
    Environment env("../resources/scenes/bunnyscene.glb", width, height);
    // Environment env("C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb", width, height);
  
    return env;
}

void updateImgData(std::vector<glm::vec3>& img_data, Environment env) {
    env.rayTrace(img_data);
}