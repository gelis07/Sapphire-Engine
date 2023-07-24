#pragma once
#include <iostream>
#include "Views.h"
#include <array>

class Engine{
    public:
        Engine(std::string Path);
        void Run();
    private:
        float LastTime;
        glm::vec4 BackgroundColor;
        int state;
        Viewport::Views viewport;
};