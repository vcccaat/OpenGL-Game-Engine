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
    Material(glm::vec3 diffuse);
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

struct BoneWeight {
public:
    int boneId;
    float weight;
};

struct BoneMat {
public:
    int boneId;
    glm::mat4 mat;
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
};


class Pipeline : public nanogui::Screen {
public:
    Pipeline(std::string path, float windowWidth, float windowHeight);
    std::string GlobalPath;

    /* init scene, lights, materials, bones, add mesh to scene */
    void initScene(std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight);
    std::vector<Material> parseMaterials(const aiScene* scene);
    std::vector<Light> parseLights(aiNode* rootNode, const aiScene* scene);
    void traverseNodeHierarchy(const aiScene* obj, aiNode* cur, glm::mat4 transmat);
    void extractBonesforVertices(aiMesh* msh);
    void addMeshToScene( aiMesh* msh, glm::mat4 transmat);
    

    /* traverse node to update model matrix in animation loop */
    void playMeshAnimation();
    void traverseTree(const aiScene* obj, aiNode* node, glm::mat4 transMat, float t);

    /* camera controls */
    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
    
    /* shading */
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

    /* each mesh has list of vertices and each vertice has vec3 postion */
    std::vector<std::vector<glm::vec3>> positions;
    std::vector<std::vector<uint32_t>> indices;
    std::vector<std::vector<glm::vec3>> normals;


    /* map a mesh's model matrix to mesh's name, 
    because when we read animation node, they might
    appear as different order of mesh node  */
    std::map<std::string, glm::mat4> transMatVec;

    /* each vertice in a mesh influenced by up to four bones under certain weights */  
    std::vector<std::vector<glm::ivec4>> boneIds;
    std::vector<std::vector<glm::vec4>> boneWts;

    /* map a vertex index to a list of bones that influence the vertex */
    std::map<int, std::vector<BoneWeight>> vertexBoneMap;

    /* map a bone name to bone trans matrix */
    std::map<std::string, BoneMat> boneTransMap;
    // std::map<int, std::map<float, int>> boneInfoMap; map is sorted, can get the largest weight 

    /*  map a bone's index to its transformation matrix */
    std::vector<glm::mat4> boneTrans;

    /* map a mesh's index to mesh's name */
    std::vector<std::string> idToName;

    /* map a mesh's index to mesh's material index */
    std::vector<int> meshIndToMaterialInd;

    /* transform camera pos to world space */
    glm::mat4 camTransMat;
    
    /* map a node's name to node's animation */
    std::map<std::string, NodeAnimate> animationOfName;

    double startTime;
    double totalTime;
    bool playAnimation;

    /* Bone matrices */
	std::vector<glm::mat4> boneMatrices;

};



