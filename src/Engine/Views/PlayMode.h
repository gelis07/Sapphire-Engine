#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include "Engine/Scenes.h"
#include "Graphics/Renderer/FrameBuffer.h"
#include <GLFW/glfw3.h>
class PlayMode{
    public:
        void Init(Scene* activeScene);
        void Render(std::string& MainPath);
        bool GameRunning = false;
        bool Paused = false;
        std::shared_ptr<Object> CameraObject;
    private:
        SapphireRenderer::FrameBuffer FrameBuffer;
        std::string m_SceneFileName;
        int m_WindowWidth, m_WindowHeight;
        Scene* m_ActiveScene;
        GLFWwindow* m_Window;
        bool m_Start = true;
};