#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include "Engine/Scenes.h"
#include <GLFW/glfw3.h>
class PlayMode{
    public:
        void Init(Scene* activeScene);
        void Render(std::string& MainPath);
        bool ImGuiRender(unsigned int texture, std::string& MainPath);
        bool Paused = true;
        std::shared_ptr<Object> CameraObject;
    private:
        unsigned int m_Texture;
        unsigned int m_FBO;
        ImVec2 ScaleWindow();
        Scene* m_ActiveScene;
        GLFWwindow* m_Window;
        bool m_Start = true;
};