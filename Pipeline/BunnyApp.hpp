#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Framebuffer.hpp>
#include <GLWrap/Mesh.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <assimp/scene.h>          // Output data structure

float getAspect(std::string path);

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

class BunnyApp : public nanogui::Screen {
public:

    void initScene(std::string path, std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight);
    std::vector<Material> parseMats(const aiScene* scene);
    std::vector<Light> parseLights(aiNode* rootNode, const aiScene* scene);
    void traverseNodeHierarchy(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals, const aiScene* obj, aiNode* cur, std::vector<glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp);
    void addMeshToScene(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices, std::vector<std::vector<glm::vec3>>& normals, aiMesh* msh, std::vector<glm::mat4>& translist, glm::mat4 transmat, std::vector<int>& mp);
    BunnyApp(std::string path, float windowWidth, float windowHeight);

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

    std::unique_ptr<GLWrap::Program> prog, fsqProg, gProg, lightProg, shadowProg, ambProg;
    std::vector<std::unique_ptr<GLWrap::Mesh>> meshes;
    std::unique_ptr<GLWrap::Mesh> fsqMesh;
    std::unique_ptr<GLWrap::Framebuffer> fbo, gfbo, lightfbo, shadowfbo;
    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::shared_ptr<RTUtil::PerspectiveCamera> lightPers;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    std::vector<Material> materials;
    std::vector<Light> lights;

    nanogui::Color backgroundColor;

    std::vector<glm::mat4> transMatVec;
    glm::mat4 camTransMat;
    std::vector<int> meshIndToMaterialInd;
};


