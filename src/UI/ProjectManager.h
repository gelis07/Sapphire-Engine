#pragma once
#include "Windows.h"
#include <GLFW/glfw3.h>
#include <filesystem>

namespace fs = std::filesystem;

class ProjectManager{
    public:
        ProjectManager();
        std::string Run();
    private:
        void SaveJson(std::string Name);
        GLFWwindow* window;
        ImGuiIO* IO;
        std::string Path;
};