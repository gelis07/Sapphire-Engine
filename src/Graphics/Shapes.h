#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"
#include "stb_image.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
//The shapes class needs a refrence to an object because it neeeds to access its position and scale
class Object;


struct Vertex{
    float Pos[2];
};
struct TextureAtlas{
    glm::vec2 IconSize;
    glm::vec2 AtlasSize;
    SapphireRenderer::Texture AtlasID;
};

namespace Shapes
{
    inline SapphireRenderer::Shader BasicShader;
    inline SapphireRenderer::Shader CircleShader;
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Shape{
        public:
            Shapes::Type ShapeType = Shapes::Null;
            bool& Wireframe() {return m_Wireframe;}
            Shape(SapphireRenderer::Shader& shader);
            // That's the function that actually render's a shape
            void RenderShape(Object* Object, std::vector<Vertex> vertices, const glm::vec3 &CamPos, float CameraZoom,bool OutLine ,bool WireFrame, std::function<void(SapphireRenderer::Shader& shader)> SetUpUniforms,bool Viewport = true);

            // Here is a virtual Render() function for every sub class to do it's own calculations before passing in the data on RenderShape()
            virtual void Render(Object* Object,const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) {} 
        private:
            SapphireRenderer::Shader& Shader;
            SapphireRenderer::VertexBuffer VertexBuffer;
            SapphireRenderer::VertexArray VertexArray;
            SapphireRenderer::IndexBuffer IndexBuffer;
            glm::mat4 m_Projection;
            bool m_Wireframe = false;
    };

    class Rectangle : public Shape
    {
        public:
            std::array<glm::vec2, 4> Points;
            Rectangle(SapphireRenderer::Shader& shader) : Shape(shader) {ShapeType = RectangleT;}
            void Render(Object* Object,const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    class Circle : public Shape
    {
        public:
            Circle(SapphireRenderer::Shader& shader) : Shape(shader) {ShapeType = CircleT;}
            void Render(Object* Object,const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    //To display the camera gizmo like portion of the screen.
    class CameraGizmo : public Shape
    {
        public:
            CameraGizmo(SapphireRenderer::Shader& shader) : Shape(shader) {ShapeType = RectangleT;}
            void Render(Object* Object,const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    

}

