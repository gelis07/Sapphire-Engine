#pragma once
#include "UI/Windows.h"
#include "Graphics/Grid.h"
#include "Engine/Scenes.h"
#include "Graphics/Renderer/FrameBuffer.h"
#include <GLFW/glfw3.h>


constexpr glm::vec4 BackgroundColor = glm::vec4(0,0,0,1);

struct ViewportCamera{
    float Zoom = 1;
    glm::vec3 position = glm::vec3(0);
};

class SceneEditor{
    public:
        void Render();
        void Init(Scene* activeScene);
        ViewportCamera ViewCamera; //This Camera is only for the m_Viewport NOT the game's camera
        std::shared_ptr<Object> SelectedObj;
        glm::vec2 GetWindowSize() {return glm::vec2(m_WindowWidth, m_WindowHeight);}
    private:
        Grid m_Grid;
        SapphireRenderer::FrameBuffer FrameBuffer;
        ImGuiContext* m_ViewportContext;
        Scene* m_ActiveScene;
        int m_WindowWidth; 
        int m_WindowHeight;
        GLFWwindow* m_Window;
        std::shared_ptr<Object> OnClick(GLFWwindow* window, std::vector<std::shared_ptr<Object>> Objects, glm::vec2&& WindowPosition); 
        void MoveCamera(glm::vec2&& Size, glm::vec2&& Position);
        static void Zooming(GLFWwindow* window, double xoffset, double yoffset);
        bool m_ClickedOnObj;
        bool m_FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
        glm::vec2 m_Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj

        ImGuizmo::OPERATION m_Operation = ImGuizmo::OPERATION::TRANSLATE;
};
