#pragma once
#include <iostream>
#include "Views/SceneEditor.h"
#include "Views/PlayMode.h"
#include <array>

class Engine{
    public:
        Engine(std::string Path);
        void Run();
    private:
        Scene ActiveScene;
        float DeltaTime;
        float LastTime;
        glm::vec4 BackgroundColor;
        GLFWwindow* MainWindow;
        Windows windows;
        SceneEditor viewport;
        PlayMode playMode;
};