#pragma once
#include <iostream>
#include "Graphics/Renderer/FrameBuffer.h"
#include "Engine/Engine.h"
#include "Imgui/ImGuizmo.h"
#include "Graphics/Grid.h"
#include "ImGuiWindows.h"
#include <array>

class Editor : public Application{
    public:
        Editor(const Editor&) = delete;
        Editor(const std::string& mainPath = "");
        void OnResize(GLFWwindow* window, int width, int height) override;
        void OnWindowFocus(GLFWwindow* window, int focused) override;
        static const std::string& GetMainPath();
        inline static const float& GetDeltaTime() {return DeltaTime;}
        static void InitWindow(std::string&& WindowName, bool state = true);
        static void SetWindowState(std::string&& WindowName, bool state);
        static bool* GetWindowState(std::string&& WindowName);
        inline static std::string CurrentPath;
        inline static std::unordered_map<std::string, SapphireEngine::Variable*> UserPreferences;
        void OnUpdate(const float DeltaTime) override;
        void OnStart() override;
        void OnExit() override;
        inline static int SelectedObjID = -1;
        inline static glm::vec2 WindowPos;
        inline static glm::vec2 WindowSize;
        inline static bool GameRunning = false;
        inline static bool Paused = false;
        inline static glm::vec2 GetWindowSize() {return glm::vec2(WindowWidth, WindowHeight);}
        inline static std::unordered_map<std::string, bool> WindowStates = 
            {{"Preferences", false},
            {"Settings", false},
            {"Project Settings", false}};
        static SapphireEngine::String ThemeName;
        std::fstream loggingFile;
    private:
        Engine engine;
        inline static float DeltaTime;
        glm::vec4 BackgroundColor;
        inline static std::string MainPath;
        static void Zooming(GLFWwindow* window, double xoffset, double yoffset);
        SapphireRenderer::FrameBuffer ViewportFBO;
        void RenderViewport();
        inline static int WindowWidth, WindowHeight;
        void MoveCamera(glm::vec2&& Size, glm::vec2&& Position);
        ViewportCamera ViewCamera; //This Camera is only for the m_Viewport NOT the game's camera
        ImGuizmo::OPERATION m_Operation = ImGuizmo::OPERATION::TRANSLATE;
        bool m_FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
        Grid grid;
        glm::vec2 m_Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj

        SapphireRenderer::FrameBuffer PlayModeFBO;
        bool Start = true;
        void RenderPlayMode();
        std::string SceneFileName;
};
