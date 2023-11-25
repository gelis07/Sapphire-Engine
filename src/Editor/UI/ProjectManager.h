#pragma once
#include "Application/Application.h"
#include "Utilities.hpp"
#include "Graphics/Shapes.h"
#include "Editor/Variables.h"
#include "json.hpp"
#include <GLFW/glfw3.h>
#include <filesystem>

namespace fs = std::filesystem;

class ProjectManager : public Application{
    public:
        ProjectManager();
        void OnStart() override {}
        void OnUpdate(const float DeltaTime) override;
        void OnExit() override;
        std::string Path;
    private:
        void SaveJson(std::string Name);
        TextureAtlas m_IconAtlas;
        std::string m_SelectedProject;
        int ProjectsSize = 0;
        bool CloseApp = true; // If the user directly closes the app from the X button GLFW should be terminated. Thats the use of this boolean
        nlohmann::json Projects;
};