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


namespace SapphireRenderer
{
    struct KeyFramePair{
        float TimeStamp =0.0f;
        std::vector<Vertex> vertices;
        std::string* path;
        void serializeString(std::ostream& file) {
            file << *path << '\n';
        }

        void deserializeString(std::istream& file) {
            path = new std::string();
            std::getline(file, *path);
        }

        void serializeVertices(std::ostream& file) {
            size_t numVertices = vertices.size();
            file.write(reinterpret_cast<const char*>(&numVertices), sizeof(size_t));
            for (const Vertex& vertex : vertices) {
                file.write(reinterpret_cast<const char*>(&vertex), sizeof(Vertex));
            }
        }

        void deserializeVertices(std::istream& file) {
            size_t numVertices;
            file.read(reinterpret_cast<char*>(&numVertices), sizeof(size_t));
            for (size_t i = 0; i < numVertices; ++i) {
                Vertex vertex;
                file.read(reinterpret_cast<char*>(&vertex), sizeof(Vertex));
                vertices.push_back(vertex);
            }
        }
    };
    struct KeyFrame{
        float TimeStamp = 0.0f;
        std::string path;
    };
    struct KeyFrameData{
        float TimeStamp = 0.0f;
        unsigned char* Data;
        float width, height, channels;
    };
    inline SapphireRenderer::Shader BasicShader;
    inline SapphireRenderer::Shader CircleShader;
    inline SapphireRenderer::Shader AnimationShader;
    inline SapphireRenderer::Shader TextureShader;
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Shape{
        public:
            SapphireRenderer::Type ShapeType = SapphireRenderer::Null;
            bool& Wireframe() {return m_Wireframe;}
            Shape(const SapphireRenderer::Shader& shader, std::vector<Vertex>&& Vertices, const std::string& path = "");
            // That's the function that actually render's a shape
            void RenderShape(const Transform& transform,const glm::vec4& Color,const glm::vec3 &CamPos, const glm::mat4& view, float CameraZoom, bool OutLine ,bool WireFrame, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms);
            void Load(const std::string& path, bool flip = false);
            void SetShader(const SapphireRenderer::Shader& shader) {Shader = shader;}
            void DeleteTexture() {Texture = std::nullopt;}
            // Here is a virtual Render() function for every sub class to do it's own calculations before passing in the data on RenderShape()
            virtual void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) {} 
            const glm::vec2 GetTextureDimensions() const;
        protected:
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
            Rectangle(const SapphireRenderer::Shader& shader, const std::string& path = "");
            void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) override;
    };
    class Circle : public Shape
    {
        public:
            Circle(const SapphireRenderer::Shader& shader);
            void Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame) override;
    };
}

