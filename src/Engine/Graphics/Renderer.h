#pragma once
#include "Utilities.hpp"
#include "Shapes.h"

class Renderer : public Component
{
    public:
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId, LuaComp), Color("Color", Variables), TexturePath("Path", Variables) {
            Color.Get() = glm::vec4(1);
            TexturePath.Get() = "";
            TexturePath.ShowOnInspector(false);
            {
                std::function<void()> OnChange = [this]() {
                    shape->Load(TexturePath.Get(), true);
                };
                TexturePath.SetOnChangeFunc(OnChange);
            }
            Functions["Play"] = LoadTexture;
        };
        ~Renderer();
        std::shared_ptr<SapphireRenderer::Shape> shape;
        SapphireRenderer::Type Type;
        SapphireEngine::Color Color;
        SapphireEngine::String TexturePath;
        void Render(const Transform& transform, const glm::mat4& view,bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom);
        void CustomRendering() override;
        static int LoadTexture(lua_State* L);
};