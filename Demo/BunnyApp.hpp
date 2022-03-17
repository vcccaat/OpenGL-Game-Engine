#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Mesh.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <assimp/scene.h>          // Output data structure

float getAspect(std::string path);

class BunnyApp : public nanogui::Screen {
public:

    BunnyApp(std::string path, float windowWidth, float windowHeight);

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;

    virtual void draw_contents() override;
    void initScene(std::string path, std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight);
    void addMeshToScene(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices,aiMesh* msh, std::vector<glm::mat4>& translist, glm::mat4 transmat);
    void traverseNodeHierarchy(std::vector<std::vector<glm::vec3>>& positions, std::vector<std::vector<uint32_t>>& indices,const aiScene* obj, aiNode* cur, std::vector<glm::mat4>& translist, glm::mat4 transmat);

private:

    static int windowWidth;
    static const int windowHeight;

    std::unique_ptr<GLWrap::Program> prog;
    std::vector<std::unique_ptr<GLWrap::Mesh>> meshes;

    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    nanogui::Color backgroundColor;

    std::vector<glm::mat4> transMatVec;
    glm::mat4 camTransMat;
    
};


