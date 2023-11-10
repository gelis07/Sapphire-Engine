#pragma once
#include "Imgui/imgui_markdown.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui_stdlib.h"
#include "Imgui/imconfig.h"
#include "Imgui/ImGuizmo.h"
#include "Utilities.hpp"
#include "Graphics/Shapes.h"
#include "Scripting/Variables.h"
#include "json.hpp"

class Windows{
    public:
        void Init(std::string&& Path);
        void Toolbar();
        void DockSpace();
        void LogWindow();
        void ThemeMaker();
        void TestWindow();
        void PerformanceWindow();
        void InitWindow(std::string&& WindowName, bool state = true);
        void SetWindowState(std::string&& WindowName, bool state);
        bool* GetWindowState(std::string&& WindowName);
        std::string MainPath;
        std::string CurrentPath; // The difference is that is the path for the file explorer.
        ImGuiContext* GetContext() {return DefaultContext;}
        ImGuiIO* GetWindowIO() {return IO;}
        inline static std::unordered_map<std::string, SapphireEngine::Variable*> SettingsVariables;
        inline static std::unordered_map<std::string, SapphireEngine::Variable*> UserPreferences;
    private:
        ImGuiContext* DefaultContext;
        ImGuiIO* IO;
        std::map<ImGuiCol_, ImVec4> LoadedTheme;
        void FileMenu();
        void Save(std::string name);
        void Load(std::string name);
        void EditMenu();
        void HelpMenu();
        void ViewMenu();
        void OnThemeChange();
        static SapphireEngine::String ThemeName;
        template <typename T>
        void SaveProjectVariable(std::string&& path, std::string&& VariableName, T data);
        float SatAmount;
        
        std::unordered_map<std::string, bool> WindowStates = 
            {{"Preferences", false},
            {"Settings", false},
            {"Project Settings", false}};
            
        void PreferencesWindow();
        void ProjectSettings();
        void Settings();
};