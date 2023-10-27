#pragma once
#include "Utilities.hpp"
#include "Shapes.h"

class Renderer : public Component
{
    public:
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId, obj, LuaComp), Color("Color", Variables) {
            Color.Get() = glm::vec4(1);
        };
        std::shared_ptr<Shapes::Shape> shape;
        Shapes::Type Type;
        SapphireEngine::Color Color;
        void Render(Transform& transform,bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom);
};