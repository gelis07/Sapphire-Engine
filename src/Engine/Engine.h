#pragma once
#include <iostream>
#include "Views/SceneEditor.h"
#include "Views/PlayMode.h"
#include <array>

class Engine{
    public:
        Engine(const Engine&) = delete;
        static Engine& Get() {return Instance;}
        void Init(std::string Path);
        void Run();
        Scene* GetActiveScene();
        const std::string& GetMainPath();
        SceneEditor& GetViewport() {return m_Viewport;}
    private:
        Engine(){}
        float DeltaTime;
        float LastTime;
        glm::vec4 BackgroundColor;
        Scene m_ActiveScene;
        GLFWwindow* m_Window;
        Windows m_Windows;
        SceneEditor m_Viewport;
        PlayMode m_PlayMode;
        static Engine Instance;
};
