#pragma once
#include "Utilities.hpp"
#include "ImGuiWindow.h"
#include "Engine/Engine.h"
#include "UI/Viewport.h"


class GamePreview : public WindowFBO{
    public:
        GamePreview(Application* app);
        void Draw() override; 
        void Extras() override;
        inline static bool GameRunning = false;
    private:
        void OnGameRestart();
        Engine engine;
        bool Start = true;
        inline static bool Paused = false;
        GLFWwindow* window;
        std::string SceneFileName;
};