// Copyright 2009-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <embree3/rtcore.h>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <iostream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <glm/glm.hpp>
#include <../RTUtil/output.hpp>
#include <../RTUtil/conversions.hpp>

//#define STBI_MSC_SECURE_CRT
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cpplocate/cpplocate.h>
#include <GLWrap/Program.hpp>

#if defined(_WIN32)
#  include <conio.h>
#  include <windows.h>
#endif

#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif


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

    Camera(aiCamera* cam) {
        this->pos = glm::vec3(cam->mPosition.x, cam->mPosition.y, cam->mPosition.z);
        this->target = glm::vec3(cam->mLookAt.x, cam->mLookAt.y, cam->mLookAt.z);
        this->up = glm::vec3(cam->mUp.x, cam->mUp.y, cam->mUp.z);
        this->hfov = cam->mHorizontalFOV;
        this->aspect = cam->mAspect;
        std::cout << "init camera" << this->pos << this->target << this->up << this->hfov << this->aspect <<std::endl;
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


void errorFunction(void* userPtr, enum RTCError error, const char* str) {
  printf("error %d: %s\n", error, str);
}

RTCDevice initializeDevice() {
  RTCDevice device = rtcNewDevice(NULL);
  if (!device) printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));
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

aiNode* findNodeWithMesh(aiNode* cur, int meshIndex) {
    for(int i = 0; i < cur->mNumMeshes; ++i) {
        if (cur->mMeshes[i] == meshIndex) return cur;
    }
    for (int i = 0; i < cur->mNumChildren; ++i) {
        return findNodeWithMesh(cur->mChildren[i], meshIndex);
    }
    // Nothing was found
    return nullptr;
}


/*
 * Create a scene, which is a collection of geometry objects. Scenes are 
 * what the intersect / occluded functions work on. You can think of a 
 * scene as an acceleration structure, e.g. a bounding-volume hierarchy.
 *
 * Scenes, like devices, are reference-counted.
 */
RTCScene initializeScene(RTCDevice device, const aiScene* aiscene, Camera &cam) {

  // Dealing with node transformations
  aiNode* rootNode = aiscene->mRootNode;
  aiCamera* camera = aiscene->mCameras[0]; //get the first camera
  aiNode* tempNode = rootNode->FindNode(camera->mName);
  glm::mat4 cmt = glm::mat4(1.f);
  glm::mat4 cur;

  // Iterate through all nodes
  while (tempNode != NULL) {
      cur = RTUtil::a2g(tempNode->mTransformation);
      std::cerr << cur << std::endl;
      cmt = cmt * cur; // this is (i think) matrix multiplation, may be wrong
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
  aiMesh** mesh = aiscene->mMeshes;
  RTCScene scene = rtcNewScene(device);

  /* 
   * Create a triangle mesh geometry, and initialize a single triangle.
   * You can look up geometry types in the API documentation to
   * find out which type expects which buffers.
   *
   * We create buffers directly on the device, but you can also use
   * shared buffers. For shared buffers, special care must be taken
   * to ensure proper alignment and padding. This is described in
   * more detail in the API documentation.
   */
  
  // add multiple meshes, bunny + floor
    for (int m = 1; m < aiscene->mNumMeshes; m++){
        printf("hi");

        RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        float* vertices = (float*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
            3*sizeof(float), 3*mesh[m]->mNumVertices);
        unsigned* indices = (unsigned*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
            3*sizeof(unsigned), 3*mesh[m]->mNumFaces);

        glm::mat4 cmt = glm::mat4(1.f);
        //aiNode* tempNode = rootNode->FindNode(mesh[m]->mName);
        aiNode* tempNode = findNodeWithMesh(rootNode, m);
        while (tempNode != rootNode) {
            cur = RTUtil::a2g(tempNode->mTransformation); // cast does not flip axes, i hope
            std::cerr << cur << std::endl;
            cmt = cmt * cur; // this is (i think) matrix multiplation, may be wrong
            tempNode = tempNode->mParent;
        }

        // Inserting mesh data into vertices
        for (int i = 0; i < mesh[m]->mNumVertices; ++i) {
            float x = mesh[m]->mVertices[i][0];
            float y = mesh[m]->mVertices[i][1];
            float z = mesh[m]->mVertices[i][2];
            chg = glm::vec4(x, y, z, 1) * cmt;
            vertices[3 * i + 0] = chg.x;
            vertices[3 * i + 1] = chg.y;
            vertices[3 * i + 2] = chg.z;
        }
        for (int i = 0; i < mesh[m]->mNumFaces; ++i) {
            indices[3 * i + 0] = mesh[m]->mFaces[i].mIndices[0];
            indices[3 * i + 1] = mesh[m]->mFaces[i].mIndices[1];
            indices[3 * i + 2] = mesh[m]->mFaces[i].mIndices[2];
        }

        /*
        * You must commit geometry objects when you are done setting them up,
        * or you will not get any intersections.
        */
        rtcCommitGeometry(geom);

        /*
        * In rtcAttachGeometry(...), the scene takes ownership of the geom
        * by increasing its reference count. This means that we don't have
        * to hold on to the geom handle, and may release it. The geom object
        * will be released automatically when the scene is destroyed.
        *
        * rtcAttachGeometry() returns a geometry ID. We could use this to
        * identify intersected objects later on.
        */
        unsigned int geomID = rtcAttachGeometry(scene, geom);
        rtcReleaseGeometry(geom);
    }

    /*
    * Like geometry objects, scenes must be committed. This lets
    * Embree know that it may start building an acceleration structure.
    */
    rtcCommitScene(scene);
    return scene;
}

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
Color castRay(RTCScene scene, 
             float ox, float oy, float oz,
             float dx, float dy, float dz)
{
  /*
   * The intersect context can be used to set intersection
   * filters or flags, and it also contains the instance ID stack
   * used in multi-level instancing.
   */
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  /*
   * The ray hit structure holds both the ray and the hit.
   * The user must initialize it properly -- see API documentation
   * for rtcIntersect1() for details.
   */
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

  /*
   * There are multiple variants of rtcIntersect. This one
   * intersects a single ray with the scene.
   */
  rtcIntersect1(scene, &context, &rayhit);

  //printf("%f, %f, %f: ", ox, oy, oz);`        
  if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
  {
      /* Note how geomID and primID identify the geometry we just hit.
       * We could use them here to interpolate geometry information,
       * compute shading, etc.
       * Since there is only a single triangle in this scene, we will
       * get geomID=0 / primID=0 for all hits.
       * There is also instID, used for instancing. See
       * the instancing tutorials for more information */
      glm::vec3 col = glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
      float out = glm::dot(glm::normalize(col), glm::normalize(glm::vec3(1.f, 1.f, -1.f)));
      out = out < 0 ? 0 : out;
      return Color(out);
  }
  return Color();
}


/**************************************** MAIN ****************************************/


int main() {
    Assimp::Importer importer;
    // Paths: C:/Users/Ponol/Documents/GitHub/Starter22/resources/meshes/bunny.obj
    //            ../resources/meshes/bunny.obj
    //        C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb
    const aiScene* obj = importer.ReadFile("C:/Users/Ponol/Documents/GitHub/Starter22/resources/scenes/bunnyscene.glb",
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);
    RTCDevice device = initializeDevice();
    aiCamera* rawcam = obj->mCameras[0];
    Camera cam = Camera();
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