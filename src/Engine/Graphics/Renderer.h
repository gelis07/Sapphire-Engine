#pragma once
#include "Utilities.hpp"
#include "Shapes.h"

class Renderer : public Component
{
    public:
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false);
        Renderer(const Renderer& renderer);
        ~Renderer();
        std::shared_ptr<SapphireRenderer::Shape> shape;
        SapphireRenderer::Type Type;
        SapphireEngine::Color Color;
        SapphireEngine::String TexturePath;
        void Render(const Transform& transform, const glm::mat4& view,bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom);
        void CustomRendering() override;
        static int LoadTexture(lua_State* L);
        static int SetColor(lua_State* L);
};