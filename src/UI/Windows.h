#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <cstring>
#include "Imgui/imgui_markdown.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imconfig.h"
#include <iostream>
#include "Utilities.hpp"
#include "Graphics/Shapes.h"

namespace Windows{
    void Init(std::string Path);
    void DockSpace();
    void LogWindow();
    inline std::string MainPath;
    inline TextureAtlas IconsAtlas;

    inline ImGuiContext* DefaultContext;
    inline ImGuiIO* IO;

    //* Set up the Themes
};