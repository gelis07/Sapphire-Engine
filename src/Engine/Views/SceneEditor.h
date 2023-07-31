#pragma once
#include "UI/Windows.h"
#include "Graphics/Grid.h"
#include "Engine/Scenes.h"
#include <GLFW/glfw3.h>


constexpr glm::vec4 BackgroundColor = glm::vec4(0,0,0,1);

struct Camera{
    float Zoom = 1;
    glm::vec3 position;
};

class SceneEditor{
    public:
        void Render();
        void Init(Scene* activeScene);
        Camera ViewportCamera; //This Camera is only for the viewport NOT the game's camera
        std::shared_ptr<Object> SelectedObj;
    private:
        Grid grid;
        unsigned int FBO;
        unsigned int Texture;
        glm::vec2 TextureSize;
        glm::vec2 ViewportPos;
        ImVec2 ScaleWindow();
        bool ImGuiRender(unsigned int texture);
        ImGuiContext* ViewportContext;
        bool ClickedOnObj;
        bool FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
        bool Start = true;
        glm::vec2 Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj

        Scene* ActiveScene;
        GLFWwindow* Window;

        void MoveCamera();
        static void Zooming(GLFWwindow* window, double xoffset, double yoffset);
};
