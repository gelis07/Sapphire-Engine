#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Imgui/imgui_markdown.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui_stdlib.h"
#include "Imgui/imconfig.h"
#include "Imgui/ImGuizmo.h"
#include "Utilities.hpp"

class Application
{
    public:
        Application(const glm::vec2& WindowDim, bool fullscreen,const std::string& Path = "");
        void Update();
        const float& GetDeltaTime() const {return DeltaTime;}
        const std::string& GetMainPath() const {return AppMainPath;}
        GLFWwindow* GetWindow() { return window;}
        ~Application();
        void Exit();
        bool GetInput(int Key);
        bool GetMouseInput(int MouseButton);
        bool GetInputDown(int Key);
        bool GetMouseInputDown(int MouseButton);
        void SetClearColor(const glm::vec4& clearColor) {ClearColor = clearColor;}
    protected:
        GLFWwindow* window;
        glm::vec4 ClearColor = glm::vec4(0);
        float DeltaTime;
        float LastTime;
        std::unordered_map<int, bool> Keys;
        std::unordered_map<int, bool> MouseButtons;
        std::string AppMainPath;
        virtual void OnResize(GLFWwindow* window, int width, int height) {}
        virtual void OnWindowFocus(GLFWwindow* window, int focused) {}
        static void OnWindowFocusCallBack(GLFWwindow* window, int focused);
        static void OnResizeCallBack(GLFWwindow* window, int width, int height);
        virtual void OnStart() {};
        virtual void OnUpdate(const float DeltaTime) {}; 
        virtual void OnExit() {};

};