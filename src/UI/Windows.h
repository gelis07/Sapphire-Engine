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

class Windows{
    public:
        void Init(std::string&& Path);
        void DockSpace();
        void LogWindow();
        std::string MainPath;
        ImGuiContext* GetContext() {return DefaultContext;}
        ImGuiIO* GetWindowIO() {return IO;}
    private:
        ImGuiContext* DefaultContext;
        ImGuiIO* IO;

    //* Set up the Themes
};

const std::string& GetMainPath();