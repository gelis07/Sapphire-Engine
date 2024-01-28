#pragma once
#include "Utilities.hpp"
#include "Objects/Transform.h"
#include "Animation.h"

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
        std::shared_ptr<Transform> transform;
        Renderer(const SapphireRenderer::Shader& shader, const std::vector<Vertex>& Vertices,const std::vector<GLuint>& Indices,SapphireRenderer::Type st,  const std::string& path = "");
        Renderer(const Renderer& renderer);
        ~Renderer();
        SapphireEngine::Color Color;
        SapphireEngine::String TexturePath;
        void CustomRendering() override;
        static int LoadTexture(lua_State* L);
        static int SetColor(lua_State* L);
        static void Render(Camera* cam, const std::vector<std::shared_ptr<Renderer>>& renderers);
        inline static std::vector<std::shared_ptr<Renderer>> SceneRenderers;
        inline static std::vector<std::shared_ptr<Renderer>> Gizmos;
        void Load(const std::string& path, bool flip = false);
        void SetShader(const SapphireRenderer::Shader& shader, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms) {Shader = shader;}
        const glm::vec2 GetTextureDimensions() const { return Texture.GetDimensions(); }
        void SelectAnimation(const std::string& name);
        std::unordered_map<std::string,SapphireRenderer::Animation*> Animations;
    public:
        std::optional<SapphireRenderer::Animation> CurrentAnimation = std::nullopt;
        std::function<void(SapphireRenderer::Shader& shader)> SetUpUniforms = [](SapphireRenderer::Shader& shader) {  };
        unsigned int IndicesAmount = 0;
        SapphireRenderer::Shader Shader;
        SapphireRenderer::VertexArray VertexArray;
        SapphireRenderer::VertexBuffer VertexBuffer;
        SapphireRenderer::IndexBuffer IndexBuffer;
        SapphireRenderer::Texture Texture;
        bool HasTexture = false;
        bool Wireframe = false;
        SapphireRenderer::Type ShapeType;
};