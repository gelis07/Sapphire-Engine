#pragma once
#include <iostream>
#include "Graphics/Renderer/FrameBuffer.h"
#include "Engine/Engine.h"
#include "Imgui/ImGuizmo.h"
#include "Graphics/Grid.h"
#include "UI/ImGuiWindow.h"
#include "UI/GamePreview.h"
#include "UI/Viewport.h"
#include "UI/Menus.h"
#include "UI/FileExplorer/FileExplorer.h"
#include <array>

class Editor : public Application{
    public:
        Editor(const Editor&) = delete;
        Editor(const std::string& mainPath = "");
        void OnResize(GLFWwindow* window, int width, int height) override;
        void OnWindowFocus(GLFWwindow* window, int focused) override;
        static const std::string& GetMainPath();
        inline static const float& GetDeltaTime() {return DeltaTime;}
        inline static std::string CurrentPath;
        inline static std::unordered_map<std::string, SapphireEngine::Variable*> UserPreferences;
        void OnUpdate(const float DeltaTime) override;
        void DuplicateObj();
        void OnStart() override;
        void OnExit() override;
        static void Export(FileExplorer& fe);
        inline static int SelectedObjID = -1;
        inline static int SelectedObjChildID = -1;
        inline static glm::vec2 WindowPos;
        inline static glm::vec2 WindowSize;
        static SapphireEngine::String ThemeName;
        static SapphireEngine::Float AASamples;
        FileExplorer fe;
    private:
        Viewport vp;
        GamePreview gp;
        Hierachy hierachy;
        LogWindow logs;
        inline static float DeltaTime;
        glm::vec4 BackgroundColor;
        inline static std::string MainPath;
        bool m_FirstTime = true; // Indicates the first time the user clicks on the SelectedObj
        glm::vec2 m_Offset; // Offset of the Cursor and the SelectedObj when the user first clicks on the SelectedObj
};
