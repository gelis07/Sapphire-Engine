#pragma once
#include "Utilities.hpp"

class Grid{
    public:
        void Init();
        void Render(glm::vec3& CameraPos, float CameraZoom);
    private:
        unsigned int GridVA;
        unsigned int GridVB;
        unsigned int GridIB;
        unsigned int GridShader;

};