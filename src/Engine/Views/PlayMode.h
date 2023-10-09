#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include "Engine/Scenes.h"
#include <GLFW/glfw3.h>
class PlayMode{
    public:
        void Init(Scene* activeScene);
        void Render(std::string& MainPath);
        bool GameRunning = false;
        bool Paused = false;
        std::shared_ptr<Object> CameraObject;
    private:
        unsigned int m_Texture;
        unsigned int m_FBO;
        GLuint m_RBO;
        std::string m_SceneFileName;
        int m_WindowWidth, m_WindowHeight;
        Scene* m_ActiveScene;
        void RescaleFrameBuffer(float width, float height);
        GLFWwindow* m_Window;
        bool m_Start = true;
};