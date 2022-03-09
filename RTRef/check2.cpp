// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <math.h>
#include <stdio.h>
#include "../RTUtil/conversions.hpp"
#include "../RTUtil/output.hpp"
#include "../RTUtil/microfacet.hpp"
#include "../RTUtil/frame.hpp"
#include <assimp/Importer.hpp>    // C++ importer interface
#include <limits>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <cpplocate/cpplocate.h>
#include <stb_image_write.h>
#include <GLWrap/Program.hpp>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>    // std::max
#include "check2.h"
#include <stdlib.h>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#endif
#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif


/**************************************** CAMERA, OTHER CONSTRUCTORS ****************************************/


Camera::Camera(aiCamera* cam, glm::vec3 tilt) {
    this->pos = glm::vec3(cam->mPosition.x, cam->mPosition.y, cam->mPosition.z);
    this->target = glm::vec3(cam->mLookAt.x, cam->mLookAt.y, cam->mLookAt.z);
    // adding tilt constant to make bunny striaght
    this->up = glm::vec3(cam->mUp.x + tilt.x, cam->mUp.y + tilt.y, cam->mUp.z + tilt.z);
    this->hfov = cam->mHorizontalFOV;
    this->aspect = cam->mAspect;
    this->phi = 1.3;
    this->theta = 1.8;
    this->dist = 8;
}

Camera::Camera() {
    this->pos = glm::vec3(3.f, 4.f, 5.f);
    this->target = glm::vec3(0.f, 0.f, 0.f);
    this->up = glm::vec3(0.f, 1.f, 0.f);
    this->hfov = 0.523599;
    this->aspect = 1.33333;
    this->phi = 1.3;
    this->theta = 1.8;
    this->dist = 8;
    this->height = .4;
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

void Camera::transformCamera() {
    pos = glm::vec3(transMat * glm::vec4(pos.x, pos.y, pos.z, 1));
    target = glm::vec3(transMat * glm::vec4(target.x, target.y, target.z, 0));
    up = glm::vec3(transMat * glm::vec4(up.x, up.y, up.z, 0));
}

void Camera::untransformCamera() {
    pos = glm::vec3(glm::inverse(transMat) * glm::vec4(pos.x, pos.y, pos.z, 1));
    target = glm::vec3(glm::inverse(transMat) * glm::vec4(target.x, target.y, target.z, 0));
    up = glm::vec3(glm::inverse(transMat) * glm::vec4(up.x, up.y, up.z, 0));
}

void Camera::recomputeSpherical() {
    this->pos.x = dist * sin(phi) * cos(theta);
    this->pos.y = dist * cos(phi);
    this->pos.z = dist * sin(phi) * sin(theta);
    this->target = -this->pos;
    this->pos += glm::vec3(0, height, 0);
    this->target += glm::vec3(0, height, 0);
}

/// nx ny is the new position of mouse after move
void Camera::orbitCamera(float nx, float ny) {

    // "Sensitivity" of mouse movement
    float pi = 3.1415926;
    float scale = .0075;
    float ep = .2;

    theta += nx * scale;
    phi -= ny * scale;
    if (phi < ep) phi = ep;
    else if (phi > pi) phi = pi;

    recomputeSpherical();
}

void Camera::zoomCamera(float ny) {

    // "Sensitivity" of mouse movement
    float scale = .02;
    float min = 2;
    float max = 25;

    dist += ny * scale;
    if (dist < min) dist = min;
    else if (dist > max) dist = max;

    recomputeSpherical();
}

void Camera::altitudeCamera(float ny) {

    // "Sensitivity" of mouse movement
    float scale = .00275;
    float min = -1;
    float max = 1.5;

    height += ny * scale;
    if (height < min) height = min;
    else if (height > max) height = max;

    recomputeSpherical();
}


Light::Light() {}

Material::Material() {
    this->diffuse = glm::vec3(.0, .5, .8);
    this->roughness = .2;
    this->indexofref = 1.5;
}


/**************************************** GIVEN FUNCTIONS ****************************************/


void errorFunction(void* userPtr, enum RTCError error, const char* str) {
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

float clip(float num, float min, float max) {
    if (num < min) return min;
    if (num > max) return max;
    return num;
}

glm::vec3 times(glm::vec3 v, float i) {
    return glm::vec3(v.x * i, v.y * i, v.z * i);
}

float toSRGB(float c) {
    if (c > 0.0031308) {
        c = 1.055 * (pow(c, (1.0 / 2.4))) - 0.055;
    }
    else {
        c = 12.92 * c;
    }
    return clip(c * 255, 0, 255);
}

/**************************************** ILLUMINATION, PARSING, NODE HIERARCHY ****************************************/


int id = 0;
void addMeshToScene(RTCDevice device, RTCScene scene, aiMesh* mesh, glm::mat4 transMatrix, std::vector<int>& mp) {
    // get vertices from aiscene meshList by mMeshes indexes
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    float* vertices = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
        3 * sizeof(float), 3 * mesh->mNumVertices);
    unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
        3 * sizeof(unsigned), 3 * mesh->mNumFaces);
    for (int i = 0; i < mesh->mNumVertices; ++i) {
        float x = mesh->mVertices[i][0];
        float y = mesh->mVertices[i][1];
        float z = mesh->mVertices[i][2];
        glm::vec4 chg = transMatrix * glm::vec4(x, y, z, 1);
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
    rtcAttachGeometryByID(scene, geom, id);
    id++;
    mp.push_back(mesh->mMaterialIndex);
    rtcReleaseGeometry(geom);
}

void traverseNodeHierarchy(RTCDevice device, RTCScene scene, const aiScene* aiscene, aiNode* cur,
    glm::mat4 transMatrix, std::vector<int>& mp) {
    // top down, compute transformation matrix while traversing down the tree
    if (cur != NULL) {
        transMatrix = RTUtil::a2g(cur->mTransformation) * transMatrix;
        // when it reaches mesh, transform the vertices
        if (cur->mNumMeshes > 0) { //&& *cur->mMeshes == 1
            for (int i = 0; i < cur->mNumMeshes; ++i) {
                aiMesh* mesh = aiscene->mMeshes[cur->mMeshes[i]];
                addMeshToScene(device, scene, mesh, transMatrix, mp);
            }
        }
        for (int i = 0; i < cur->mNumChildren; ++i) {
            traverseNodeHierarchy(device, scene, aiscene, cur->mChildren[i], transMatrix, mp);
        }
    }
}

RTCScene initializeScene(RTCDevice device, const aiScene* aiscene, Camera& cam, std::vector<int>& mp) {
    RTCScene scene = rtcNewScene(device);
    traverseNodeHierarchy(device, scene, aiscene, aiscene->mRootNode, glm::mat4(1.f), mp);
    rtcCommitScene(scene);
    return scene;
}

glm::mat4 getTransMatrix(aiNode* rootNode, aiString nodeName) {
    // Dealing with node hierarchy
    aiNode* tempNode = rootNode->FindNode(nodeName);
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

aiColor3D Light::pointIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hitPos, glm::vec3 normal, Material material) {

    float pi = 3.1415926;
    glm::vec3 lightDir = glm::normalize(pos - hitPos);

    if (isShadowed(scene, lightDir, hitPos)) return aiColor3D();

    //  wi: Incident direction from hit point to light
    //  wo: Outgoing direction from hit point to camera
    glm::vec3 wi = lightDir;
    glm::vec3 wo = glm::normalize(-eyeRay);

    normal = glm::normalize(normal);

    nori::Frame frame = nori::Frame(normal);
    nori::BSDFQueryRecord BSDFquery(frame.toLocal(wi), frame.toLocal(wo));

    nori::Microfacet bsdf = nori::Microfacet(material.roughness, material.indexofref, 1.f, material.diffuse);
    glm::vec3 fr = bsdf.eval(BSDFquery);

    glm::vec3 out = glm::vec3(power[0] * fr[0], power[1] * fr[1], power[2] * fr[2]) * std::max(0.f, glm::dot(normal, wi));

    return aiColor3D(out[0] / 255, out[1] / 255, out[2] / 255);
}

aiColor3D Light::areaIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hitPos, glm::vec3 normal, Material material) {
    float pi = 3.1415926;
    // Determine random position of light
    float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    
    glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0,1,0), areaNormal));
    // std::cout << "u " <<u <<std::endl;
    glm::vec3 v = glm::normalize(glm::cross(areaNormal,u));
    // std::cout << "v " <<v <<std::endl;
    glm::vec3 lightpos = pos + v * (r1 * width - width / 2) + u * (r2 * height - height / 2);
    // std::cout << "light " <<lightpos <<std::endl;

    // float xLocal = (r1 * width) - width / 2;
    // float yLocal = (r2 * height) - height / 2;
    // glm::vec3 lightpos = glm::vec3(xLocal, yLocal, 0);
    // lightpos = glm::vec3(transMat * glm::vec4(lightpos.x, lightpos.y, lightpos.z, 1));

    glm::vec3 lightDir = glm::normalize(lightpos - hitPos);
    if (isShadowed(scene, lightDir, hitPos)) return aiColor3D();

    // Eval BRDF and formula
    // wi wo direction not sure
    glm::vec3 wi = glm::normalize(-eyeRay);
    glm::vec3 wo = lightDir;

    normal = glm::normalize(normal);
    nori::Frame frame = nori::Frame(normal);
    nori::BSDFQueryRecord BSDFquery(frame.toLocal(wi), frame.toLocal(wo));
    nori::Microfacet bsdf = nori::Microfacet(material.roughness, material.indexofref, 1.f, material.diffuse);
    glm::vec3 fr = bsdf.eval(BSDFquery);

    glm::vec3 firstpt = glm::vec3(power[0] * fr[0], power[1] * fr[1], power[2] * fr[2]);
    float toppt = glm::dot(normal, wo) * glm::dot(areaNormal, wo);
    float bottompt = pow(glm::length(hitPos - lightpos), 2);
    glm::vec3 out = pi * width * height * firstpt * (toppt / bottompt);

    return aiColor3D(out[0] / 255, out[1] / 255, out[2] / 255);
}

aiColor3D Light::ambientIlluminate(RTCScene scene, glm::vec3 eyeRay, glm::vec3 hitPos, glm::vec3 normal, Material material) {
    float pi = 3.1415926;
    float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    glm::vec3 samp = RTUtil::squareToCosineHemisphere(glm::vec2(r1, r2));

    
    nori::Frame frame = nori::Frame(normal);
    glm::vec3 globalSamp = frame.toWorld(samp);  //direction
    // std::cout << "normal " << normal << "world sample " << globalSamp << std::endl;

    if (isShadowed(scene, globalSamp, hitPos, dist)) return aiColor3D();

    glm::vec3 out = glm::vec3(material.diffuse[0] * power[0], material.diffuse[1] * power[1], material.diffuse[2] * power[2]) * pi;

    return aiColor3D(out[0], out[1], out[2]);
}


std::vector<Light> parseLights(aiNode* rootNode, const aiScene* scene) {

    // Initial empty list, then loop over all lights
    std::vector<Light> lights = {};
    for (int i = 0; i < scene->mNumLights; i++) {
        // Construct light
        Light l = Light();
        l.name = scene->mLights[i]->mName.C_Str();
        l.sceneindex = i;

        // Parse area, ambient, and point
        if (RTUtil::parseAreaLight(l.name, l.width, l.height)) {
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse;
            l.type = l.AREA;
            l.areaNormal = glm::vec3(0, 0, 1);
            //std::cout << "Width: " << l.width << "\nHeight: " << l.width << "\nPos: " << l.pos << "\nPower: " << l.power[0];
        }
        else if (RTUtil::parseAmbientLight(l.name, l.dist)) {
            l.power = scene->mLights[i]->mColorAmbient;
            l.type = l.AMBIENT;
        }
        else {
            aiVector3D p = scene->mLights[i]->mPosition;
            l.pos = glm::vec3(p.x, p.y, p.z);
            l.power = scene->mLights[i]->mColorDiffuse;
            l.type = l.POINT;
        }

        // transform light
        glm::mat4 transMat = getTransMatrix(rootNode, scene->mLights[i]->mName);
        l.pos = glm::vec3(transMat * glm::vec4(l.pos.x, l.pos.y, l.pos.z, 1));
        l.transMat = transMat;
        if (l.type == l.AREA) {
            l.areaNormal = glm::vec3(transMat * glm::vec4(l.areaNormal.x, l.areaNormal.y, l.areaNormal.z, 0));
        }
        // Push to list
        lights.push_back(l);
    }
    if (lights.size() == 0) {
        // default pointlight, arealight, and ambientlight
        Light lp = Light();
        lp.pos = glm::vec3(8, 9, 2);
        lp.power = aiColor3D(200, 100, 300);
        lp.type = lp.POINT;
        lp.transMat = glm::mat4(1);
        lights.push_back(lp);

        Light la = Light();
        la.pos = glm::vec3(0, 0, -8);
        la.power = aiColor3D(300, 300, 300);
        la.type = la.AREA;
        la.transMat = glm::mat4(1);
        la.areaNormal = glm::vec3(0, 0, 1);
        la.width = 3;
        la.height = 3;
        //lights.push_back(la);

        Light le = Light();
        le.power = aiColor3D(.025, .015, .015);
        le.type = le.AMBIENT;
        le.dist = 600;
        lights.push_back(le);
    }
    return lights;
}

std::vector<Material> parseMats(const aiScene* scene) {
    std::vector<Material> mats = {};
    for (int i = 0; i < scene->mNumMaterials; ++i) {
        Material m = Material();
        scene->mMaterials[i]->Get(AI_MATKEY_ROUGHNESS_FACTOR, m.roughness);
        scene->mMaterials[i]->Get(AI_MATKEY_BASE_COLOR, reinterpret_cast<aiColor3D&>(m.diffuse));
        mats.push_back(m);
    }
    return mats;
}


/**************************************** ENVIRONMENT ****************************************/


Environment::Environment() {}

Environment::Environment(std::string objpath, int width, int height) {
    this->width = width;
    this->height = height;

    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(objpath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    this->device = initializeDevice();

    aiNode* rootNode = obj->mRootNode;

    //get the first camera, or default if there is none
    if (obj->mNumCameras == 0) {
        this->camera = Camera();
    }
    else {
        glm::vec3 camtilt = glm::vec3(0, 0, .0975); // constants to account for tilting. trial-and-error 
        aiCamera* camera = obj->mCameras[0];
        this->camera = Camera(camera, camtilt); // apply tilting constants to constructor
        this->camera.transMat = getTransMatrix(rootNode, camera->mName);
        this->camera.transformCamera();
    }


    //get light and set transformation matrix 
    this->lights = parseLights(rootNode, obj);
    for (int i = 0; i < lights.size(); ++i) {
        if (lights[i].type == lights[i].AMBIENT) {
            this->background = lights[i].power;
            break;
        }
    }
    this->materials = parseMats(obj);

    this->geomIdToMatInd = {};
    this->scene = initializeScene(this->device, obj, this->camera, this->geomIdToMatInd);
}

void Environment::rayTrace(std::vector<glm::vec3>& img_data, float iter) {
    glm::vec3 dir;
    for (int j = 0; j < height; ++j) for (int i = 0; i < width; ++i) {
        dir = camera.generateRay((i + .5) / width, (j + .5) / height);
        aiColor3D col = castRay(camera.pos.x, camera.pos.y, camera.pos.z, dir.x, dir.y, dir.z);
        img_data[j * width + i] = times(img_data[j * width + i], (iter - 1) / iter) + times(glm::vec3(col.r, col.g, col.b), 1 / iter);
    }
}

RTCRayHit generateRay(float ox, float oy, float oz, float dx, float dy, float dz) {
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
    return rayhit;
}

aiColor3D Environment::castRay(float ox, float oy, float oz, float dx, float dy, float dz) {
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    RTCRayHit rayhit = generateRay(ox, oy, oz, dx, dy, dz);

    rtcIntersect1(scene, &context, &rayhit);

    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID && rayhit.ray.tfar != std::numeric_limits<float>::infinity()) {
        glm::vec3 normal = glm::normalize(glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));

        // diffuse shading and specular reflectance
        glm::vec3 rayDir = glm::vec3(dx, dy, dz);
        glm::vec3 hitPos = glm::vec3(ox, oy, oz) + rayhit.ray.tfar * rayDir;
        return shade(rayDir, hitPos, normal, rayhit.hit.geomID);
    }
    return this->background;
}

bool isShadowed(RTCScene scene, glm::vec3 lightDir, glm::vec3 hitPos, float maxDist) {
    // glm::vec3 lightDir = glm::normalize(lightpos - hitPos);
    glm::vec3 newOrig = hitPos + lightDir * .001f;
    RTCRayHit shadowRayhit = generateRay(newOrig[0], newOrig[1], newOrig[2], lightDir[0], lightDir[1], lightDir[2]);
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    rtcOccluded1(scene, &context, &shadowRayhit.ray);
    if (shadowRayhit.ray.tfar > maxDist) return false;
    return shadowRayhit.ray.tfar == -std::numeric_limits<float>::infinity();
}

aiColor3D Environment::shade(glm::vec3 eyeRay, glm::vec3 hitPos, glm::vec3 normal, int geomID) {
    aiColor3D color;
    Material material = materials[geomIdToMatInd[geomID]];
    for (int i = 0; i < lights.size(); i++) {
        if (lights[i].type == 0) {
            // color = color + lights[i].pointIlluminate(scene, eyeRay, hitPos, normal, material);
        }
        else if (lights[i].type == 1) {
            color = color + lights[i].areaIlluminate(scene, eyeRay, hitPos, normal, material);
        }
        else {
            // color = color + lights[i].ambientIlluminate(scene, eyeRay, hitPos, normal, material);
        }
    }
    return color;
}


/**************************************** MAIN FUNCTIONS ****************************************/


float getAspect(std::string path) {
    Assimp::Importer importer;
    const aiScene* obj = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    if (obj->mNumCameras == 0) return 1.333333;
    return obj->mCameras[0]->mAspect;
}

Environment startup(std::string path, int width, int height) {
    Environment env(path, width, height);
    return env;
}

bool continueSaving = true;
void updateImgData(std::vector<glm::vec3>& img_data, Environment env, int iter, std::string sceneName, bool saveImg) {
    env.rayTrace(img_data, (float)iter);
    // Save image
     if (iter % 64 == 0 && saveImg && continueSaving) {
         if (iter == 256) continueSaving = false;
         unsigned char* img = new unsigned char[env.width * env.height * 3];
         int k = 0;
         for (int j = 0; j < env.height; ++j) for (int i = 0; i < env.width; ++i) {
             img[(3 * j * env.width) + (3 * i) + 0] = toSRGB(img_data[k][0]);
             img[(3 * j * env.width) + (3 * i) + 1] = toSRGB(img_data[k][1]);
             img[(3 * j * env.width) + (3 * i) + 2] = toSRGB(img_data[k][2]);
             k++;
         }
         stbi_flip_vertically_on_write(1);
         std::string name = "render_" + sceneName + "_" + std::to_string(iter) + ".png";
         stbi_write_png(name.c_str(), env.width, env.height, 3, img, env.width * 3);
         delete[] img;
         std::cout << "Wrote " << name << "\n";
     }
}