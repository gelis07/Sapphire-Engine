#pragma once
#include "Utilities.hpp"
#include "ImGuiWindow.h"
#include "Objects/Objects.h"
#include "Graphics/Grid.h"

class Viewport;
class Hierachy : public Window{
    public:
        Hierachy(Viewport* vp) : Window("Hieracy") {viewport = vp;}
        void Display() override;
    private:
        void CreateMenu(ObjectRef SelectedObj);
        Viewport* viewport;
};

class Viewport : public WindowFBO{
    public:
        Viewport();
        void Init();
        void Draw() override; 
        void Extras() override;
        ObjectRef SelectedObj;
    private:
        Grid grid;
        Camera ViewCamera; // This Camera is only for the viewport NOT the game's camera
        void MoveCamera(glm::vec2 &&Size, glm::vec2 &&Position);
        void Gizmos();
        ImGuizmo::OPERATION Operation = ImGuizmo::OPERATION::TRANSLATE;
        bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
        GLFWwindow* window;
};