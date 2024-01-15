#pragma once
#include "Utilities.hpp"
#include "Objects/Transform.h"
#include "Shapes.h"

class Camera : public Component
{
    public:
        Camera(const std::string& Name)
        : Component(Name), BgColor("BgColor", Variables), Zoom("Zoom", Variables){
            BgColor.Get() = glm::vec4(0,0,0,1);
            Zoom.Get() = 1.0f;
        };
        SapphireEngine::Color BgColor;
        SapphireEngine::Float Zoom;
        std::shared_ptr<Transform> Transform;
        glm::mat4 GetView();
};

class Renderer : public Component
{
    public:
        Transform* transform;
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false);
        Renderer(const Renderer& renderer);
        ~Renderer();
        std::shared_ptr<SapphireRenderer::Shape> shape;
        SapphireRenderer::Type Type;
        SapphireEngine::Color Color;
        SapphireEngine::String TexturePath;
        void CustomRendering() override;
        static int LoadTexture(lua_State* L);
        static int SetColor(lua_State* L);
        static void Render(Camera* cam);
        inline static std::vector<std::shared_ptr<Renderer>> Shapes;
};