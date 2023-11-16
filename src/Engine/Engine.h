#pragma once
#include <iostream>
#include "Views/SceneEditor.h"
#include "Views/PlayMode.h"
#include "Application/Application.h"
#include <array>

class Engine : public Application{
    public:
        Engine(const Engine&) = delete;
        static Engine& Get() {return Instance;}
        void OnStart() override;
        void OnUpdate(const float DeltaTime) override;
        void OnExit() override;
        void OnResize(GLFWwindow* window, int width, int height) override;
        void OnWindowFocus(GLFWwindow* window, int focused) override;
        void Export();
        Scene* GetActiveScene();
        const std::string& GetMainPath();
        SceneEditor& GetViewport() {return m_Viewport;}
        PlayMode& GetPlay() {return m_PlayMode;}
        Windows& GetWindows() {return m_Windows;}
        const float& GetDeltaTime() {return DeltaTime;}
    private:
        Engine() { }
        float DeltaTime;
        glm::vec4 BackgroundColor;
        Scene m_ActiveScene;
        GLFWwindow* m_Window;
        Windows m_Windows;
        SceneEditor m_Viewport;
        PlayMode m_PlayMode;
        static Engine Instance;
};
