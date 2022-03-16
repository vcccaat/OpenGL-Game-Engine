#pragma once

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Mesh.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <assimp/scene.h>          // Output data structure
using namespace std;

class BunnyApp : public nanogui::Screen {
public:

    BunnyApp();

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;

    virtual void draw_contents() override;
    void initScene(std::shared_ptr<RTUtil::PerspectiveCamera>& cam, float windowWidth, float windowHeight);
    void addMeshToScene(vector<glm::vec3>& positions, vector<uint32_t>& indices,aiMesh* msh, int& count, std::vector<glm::mat4>& translist, glm::mat4 transmat);
    void traverseNodeHierarchy(vector<glm::vec3>& positions, vector<uint32_t>& indices,const aiScene* obj, aiNode* cur, int& count, std::vector<glm::mat4>& translist, glm::mat4 transmat);

private:

    static const int windowWidth;
    static const int windowHeight;
    bool useDefaultCamera;

    std::unique_ptr<GLWrap::Program> prog;
    std::unique_ptr<GLWrap::Mesh> mesh;

    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    nanogui::Color backgroundColor;
};


