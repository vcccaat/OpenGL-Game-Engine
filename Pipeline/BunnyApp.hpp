#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Framebuffer.hpp>
#include <GLWrap/Mesh.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <assimp/scene.h>          // Output data structure
#include <map>
#include <chrono>
#include <ctime>   
#include <cmath>   

float getAspect(std::string path);
double getSecondsSinceEpoch();

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
    glm::vec3 pos; // currently stored in global space
    glm::vec3 areaNormal;
    glm::vec3 areaTangent;
    aiColor3D power;
    int sceneindex;
    float width;
    float height;
    float dist;
    glm::mat4 transMat;

    enum Type { POINT, AREA, AMBIENT };
    Type type;

    Light();
};

struct KeyframePos {
public:
	float time;
	glm::vec3 pos;
};
struct KeyframeRot {
public:
	float time;
	aiQuaternion rot;
};
struct KeyframeScale {
public:
	float time;
	glm::vec3 scale;
};
	

struct NodeAnimate {
public:
	std::string name;
    std::vector<KeyframePos> keyframePos;
    std::vector<KeyframeRot> keyframeRot;
    std::vector<KeyframeScale> keyframeScale;
    // std::map<float, Keyframe> keyframes;	
};

// class Node {
// public:
//     int mNumMeshes;
//     std::String mName;
// };

class BunnyApp : public nanogui::Screen {
public:
    // std::shared_ptr<Node> root;
    void initScene(std::string path, std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight);
    std::vector<Material> parseMats(const aiScene* scene);
    std::vector<Light> parseLights(aiNode* rootNode, const aiScene* scene);
    void traverseNodeHierarchy(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals,const aiScene* obj, aiNode* cur, std::map<std::string, glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp, std::vector<std::string>& itn);
    void addMeshToScene(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals, aiMesh* msh, std::map<std::string, glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp);
    BunnyApp(std::string path, float windowWidth, float windowHeight);

    void traverseTree(aiNode* node, glm::mat4 transMat, int counter, float t);

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
    virtual void forwardShade();
    virtual void deferredShade();
    virtual void draw_contents() override;

		
private:

    static int windowWidth;
    static const int windowHeight;
    bool deferred;
    bool toggle;

    std::unique_ptr<GLWrap::Program> prog, fsqProg, gProg, lightProg, shadowProg, ambProg, sunskyProg, accProg, mergeProg;
    std::vector<std::unique_ptr<GLWrap::Mesh>> meshes;
    std::unique_ptr<GLWrap::Mesh> fsqMesh;
    std::unique_ptr<GLWrap::Framebuffer> fbo, gfbo, lightfbo, shadowfbo, blurHorfbo, blurVerfbo, mergefbo;
    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::shared_ptr<RTUtil::PerspectiveCamera> lightPers;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    std::vector<Material> materials;
    std::vector<Light> lights;

    nanogui::Color backgroundColor;

    std::map<std::string, glm::mat4> transMatVec;

    glm::mat4 camTransMat;
    std::vector<int> meshIndToMaterialInd;

    std::map<std::string, NodeAnimate> animationOfName;
    double curTime;
    double startTime;
    double totalTime;
    std::vector<std::string> idToName;
    
};


