// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <assimp/postprocess.h>  // Post processing flags
#include <assimp/scene.h>        // Output data structure
#include <embree3/rtcore.h>
#include <math.h>
#include <stdio.h>

#include <../RTUtil/conversions.hpp>
#include <../RTUtil/output.hpp>
#include <assimp/Importer.hpp>  // C++ importer interface
#include <glm/glm.hpp>
#include <iostream>
#include <limits>

//#define STBI_MSC_SECURE_CRT
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <cpplocate/cpplocate.h>
#include <stb_image_write.h>

#include <GLWrap/Program.hpp>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#endif

#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif

#include "check2.h"


/**************************************** STRUCTURES ****************************************/

struct Color {
  unsigned char r;
  unsigned char g;
  unsigned char b;

  Color(unsigned char r, unsigned char g, unsigned char b) {
    this->r = r;
    this->g = g;
    this->b = b;
  }

  Color(float r, float g, float b) {
    this->r = 255 * r;
    this->g = 255 * g;
    this->b = 255 * b;
  }

  Color(unsigned char c) {
    this->r = c;
    this->g = c;
    this->b = c;
  }

  Color(float c) {
    this->r = 255 * c;
    this->g = 255 * c;
    this->b = 255 * c;
  }

  Color() {
    this->r = 0;
    this->g = 0;
    this->b = 0;
  }

  void print() {
    printf("%c, %c, %c\n", this->r, this->g, this->b);
  }
};

class Camera {
 public:
  glm::vec3 pos;
  glm::vec3 target;
  glm::vec3 up;
  float hfov;
  float aspect;

  Camera(aiCamera *cam) {
    this->pos = glm::vec3(cam->mPosition.x, cam->mPosition.y, cam->mPosition.z);
    this->target = glm::vec3(cam->mLookAt.x, cam->mLookAt.y, cam->mLookAt.z);
    this->up = glm::vec3(cam->mUp.x, cam->mUp.y, cam->mUp.z);
    this->hfov = cam->mHorizontalFOV;
    this->aspect = cam->mAspect;
    std::cout << "init camera" << this->pos << this->target << this->up << this->hfov << this->aspect << std::endl;
  }

  Camera() {
    this->pos = glm::vec3(1.f, -1.f, 6.f);
    this->target = glm::vec3(-1.5f, -1.5f, 0.f);
    this->up = glm::vec3(0.f, 1.f, 0.f);
    this->hfov = 0.5;
    this->aspect = 1;
  }

  glm::vec3 generateRay(float xp, float yp, int n) {
    glm::vec3 w = glm::normalize(this->pos - this->target);
    glm::vec3 u = glm::normalize(glm::cross(this->up, w));
    glm::vec3 v = glm::normalize(glm::cross(w, u));

    float vfov = this->hfov / (this->aspect * n);
    double h = 2 * tan(vfov / 2);
    double wide = this->aspect * h;
    double u_small = xp * wide;
    double v_small = yp * h;
    u_small -= wide / 2;
    v_small -= h / 2;
    float x = u.x * u_small + v.x * v_small - w.x;
    float y = u.y * u_small + v.y * v_small - w.y;
    float z = u.z * u_small + v.z * v_small - w.z;
    return glm::vec3(x, y, z);
  }
};

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

/**************************************** SCENE AND RAY ****************************************/

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
    glm::vec4 chg = glm::vec4(x, y, z, 1) * transMatrix;
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
  printf("finish adding mesh to scene\n");
}

void traverseNodeHierarchy(RTCDevice device, RTCScene scene, const aiScene *aiscene, aiNode *cur, glm::mat4 transMatrix) {
  // top down, compute transformation matrix while traversing down the tree
  if (cur != NULL) {
    transMatrix =  RTUtil::a2g(cur->mTransformation)*transMatrix;
    
    // when it reaches mesh, transform the vertices
    if (cur->mNumMeshes > 0 && *cur->mMeshes == 0) { /// temp: only has bunny   && *cur->mMeshes == 0
      
      aiMesh **meshList = aiscene->mMeshes;
      aiMesh *mesh = meshList[*cur->mMeshes];
      printf("index of mesh %d \n",*cur->mMeshes);
      std::cerr << transMatrix << std::endl;
      addMeshToScene(device, scene, mesh, glm::mat4(1.f)); //transMatrix  temp!!! glm::mat4(1.f)
    //   transMatrix = glm::mat4(1.f); //!!!!!
      return;
    }
    for (int i = 0; i < cur->mNumChildren; ++i) {
      traverseNodeHierarchy(device, scene, aiscene, cur->mChildren[i], transMatrix);
    }
  }
}

RTCScene initializeScene(RTCDevice device, const aiScene *aiscene, Camera &cam) {
  // Dealing with node transformations
  aiNode *rootNode = aiscene->mRootNode;
  aiCamera *camera = aiscene->mCameras[0];  //get the first camera
  aiNode *tempNode = rootNode->FindNode(camera->mName);
  glm::mat4 cmt = glm::mat4(1.f);
  glm::mat4 cur;

  // Iterate through all nodes
  while (tempNode != NULL) {
    cur = RTUtil::a2g(tempNode->mTransformation);
    std::cerr << cur << std::endl;
    cmt = cmt * cur;  // this is (i think) matrix multiplation, may be wrong
    tempNode = tempNode->mParent;
  }
  // Now, alter camera attributes
  glm::vec4 chg = glm::vec4(cam.pos.x, cam.pos.y, cam.pos.z, 1) * cmt;
  cam.pos = glm::vec3(chg);
  chg = glm::vec4(cam.target.x, cam.target.y, cam.target.z, 1) * cmt;
  cam.target = glm::vec3(chg);
  chg = glm::vec4(cam.up.x, cam.up.y, cam.up.z, 0) * cmt;
  cam.up = glm::vec3(chg);

  // Look at mesh stuff
  RTCScene scene = rtcNewScene(device);
  traverseNodeHierarchy(device, scene, aiscene, aiscene->mRootNode, glm::mat4(1.f));
  rtcCommitScene(scene);
  return scene;
}

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
Color castRay(RTCScene scene,
              float ox, float oy, float oz,
              float dx, float dy, float dz) {
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
    float out = glm::dot(glm::normalize(col), glm::normalize(glm::vec3(1.f, 1.f, -1.f)));
    out = out < 0 ? 0 : out;
    return Color(out);
  }
  return Color();
}

/**************************************** MAIN ****************************************/

int run() {
    Assimp::Importer importer;
    // Paths: C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.
    //        C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb
    //        ../resources/meshes/bunny.obj
    //        ../resources/scenes/bunnyscene.glb
    const aiScene* obj = importer.ReadFile("../resources/scenes/bunnyscene.glb",
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);
    RTCDevice device = initializeDevice();
    aiCamera* rawcam = obj->mCameras[0];
    Camera cam = Camera(rawcam); //rawcam
    RTCScene scene = initializeScene(device, obj, cam);

    // Constants
    const int n = 256;
    unsigned char img [n * n * 3];

    // New tracing with camera
    glm::vec3 dir;
    for(int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
        dir = cam.generateRay(i + .5 / n, j + .5 / n, n);
        Color col = castRay(scene, cam.pos.x, cam.pos.z, cam.pos.y, dir.x, dir.z, dir.y);
        img[(3 * j * n) + (3 * i) + 0] = col.r;
        img[(3 * j * n) + (3 * i) + 1] = col.g;
        img[(3 * j * n) + (3 * i) + 2] = col.b;
    }

  // Write the image
  stbi_write_png("bunny.png", n, n, 3, img, n * 3);
  return 0;
}


std::vector<glm::vec3> getImgData(int width, int height) {
    Assimp::Importer importer;
    // Paths: C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.
    //        C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb
    //        ../resources/meshes/bunny.obj
    //        ../resources/scenes/bunnyscene.glb
    const aiScene* obj = importer.ReadFile("../resources/scenes/bunnyscene.glb",
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);
    RTCDevice device = initializeDevice();
    aiCamera* rawcam = obj->mCameras[0];
    Camera cam = Camera(rawcam); //rawcam
    RTCScene scene = initializeScene(device, obj, cam);

    std::vector<glm::vec3> img  = std::vector<glm::vec3>(width * height,glm::vec3(0.0f));

    // New tracing with camera
    glm::vec3 dir;
    for(int i = 0; i < width; ++i) for (int j = 0; j < height; ++j) {
        dir = cam.generateRay(i + .5 / width, j + .5 / height, width);  // wierd fov ratio!!
        Color col = castRay(scene, cam.pos.x, cam.pos.z, cam.pos.y, dir.x, dir.z, dir.y);
        img[i*width + j] = glm::vec3(col.r,col.g,col.b);
        // std::cerr <<  glm::vec3(col.r,col.g,col.b) << std::endl;
    }

  return img;
}

// Static tracing (would go after "// Constants")
/*
float bottomLeftBound [2] = { -1.25, -1.25 };
float topRightBound [2] = { 1.25, 1.25 };

// Cast rays with origin in bounding box
float xstep = (topRightBound[0] - bottomLeftBound[0]) / n;
float ystep = (topRightBound[1] - bottomLeftBound[1]) / n;
float ox;
float oy;
for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
    ox = i * xstep + bottomLeftBound[0];
    oy = j * ystep + bottomLeftBound[1];
    Color col = castRay(scene, ox, oy, 1, 0, 0, -1);
    img[(3 * j * n) + (3 * i) + 0] = col.r;
    img[(3 * j * n) + (3 * i) + 1] = col.g;
    img[(3 * j * n) + (3 * i) + 2] = col.b;
}
*/