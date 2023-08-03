#pragma once
#include "UI/Windows.h"
#include "Graphics/Grid.h"
#include "Engine/Scenes.h"
#include <GLFW/glfw3.h>


constexpr glm::vec4 BackgroundColor = glm::vec4(0,0,0,1);

struct Camera{
    float Zoom = 1;
    glm::vec3 position = glm::vec3(0);
};

class SceneEditor{
    public:
        void Render();
        void Init(Scene* activeScene);
        Camera ViewportCamera; //This Camera is only for the m_Viewport NOT the game's camera
        std::shared_ptr<Object> SelectedObj;
    private:
        Grid m_Grid;
        unsigned int m_FBO;
        unsigned int m_Texture;
        glm::vec2 m_ViewportSize;
        glm::vec2 m_ViewportPosition;
        ImGuiContext* m_ViewportContext;
        bool ClickedOnObj;
        bool m_FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
        glm::vec2 m_Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj
        Scene* m_ActiveScene;
        GLFWwindow* m_Window;
        ImVec2 ScaleWindow();
        bool ImGuiRender(unsigned int texture);
        void MoveCamera();
        static void Zooming(GLFWwindow* window, double xoffset, double yoffset);
};
