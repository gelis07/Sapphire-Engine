#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include "Engine/Scenes.h"
#include <GLFW/glfw3.h>
class PlayMode{
    public:
        void Init(Scene* activeScene);
        void Render(std::string& MainPath);
        bool ImGuiRender(unsigned int texture, std::string& MainPath);
        bool Paused = true;
    private:
        unsigned int Texture; 
        unsigned int FBO;
        ImVec2 ScaleWindow();
        Scene* ActiveScene;
        GLFWwindow* Window;
        bool Start = true;
};