#pragma once
#include <iostream>
#include "Views/SceneEditor.h"
#include "Views/PlayMode.h"
#include <array>

class Engine{
    public:
        Engine(std::string Path);
        void Run();
        static Scene* GetActiveScene();
        static const std::string& GetMainPath();
    private:
        float DeltaTime;
        float LastTime;
        glm::vec4 BackgroundColor;
        Scene m_ActiveScene;
        GLFWwindow* m_Window;
        Windows m_Windows;
        SceneEditor m_Viewport;
        PlayMode m_PlayMode;
};