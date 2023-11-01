#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"
#include "stb_image.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Objects/Transform.h"
//The shapes class needs a refrence to an object because it neeeds to access its position and scale
class Object;


struct Vertex{
    glm::vec2 Pos;
    glm::vec2 TextureCoord;
};
struct TextureAtlas{
    glm::vec2 IconSize;
    glm::vec2 AtlasSize;
    SapphireRenderer::Texture AtlasID;
};

struct ViewportCamera{
    float Zoom = 1;
    glm::vec3 position = glm::vec3(0);
    glm::mat4 View = glm::mat4(0.0f);
};


namespace Shapes
{
    inline SapphireRenderer::Shader BasicShader;
    inline SapphireRenderer::Shader CircleShader;
    inline SapphireRenderer::Shader TextureShader;
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Shape{
        public:
            Shapes::Type ShapeType = Shapes::Null;
            bool& Wireframe() {return m_Wireframe;}
            Shape(const SapphireRenderer::Shader& shader, const std::string& path = "");
            // That's the function that actually render's a shape
            void RenderShape(const Transform& transform,const glm::vec4& Color,const glm::vec3 &CamPos, const glm::mat4& view, float CameraZoom, bool OutLine ,bool WireFrame, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms);
            void Load(const std::string& path, bool flip = false);
            // Here is a virtual Render() function for every sub class to do it's own calculations before passing in the data on RenderShape()
            virtual void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) {} 
        private:
            SapphireRenderer::Shader Shader;
            SapphireRenderer::VertexBuffer VertexBuffer;
            SapphireRenderer::VertexArray VertexArray;
            SapphireRenderer::IndexBuffer IndexBuffer;
            std::optional<SapphireRenderer::Texture> Texture;
            glm::mat4 m_Projection;
            bool m_Wireframe = false;
    };

    class Rectangle : public Shape
    {
        public:
            Rectangle(const SapphireRenderer::Shader& shader, const std::string& path = "") : Shape(shader, path) {ShapeType = RectangleT;}
            void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) override;
    };
    class Circle : public Shape
    {
        public:
            Circle(const SapphireRenderer::Shader& shader) : Shape(shader) {ShapeType = CircleT;}
            void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) override;
    };
}

