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

class Camera;
namespace SapphireRenderer
{
    struct KeyFramePair{
        float TimeStamp =0.0f;
        std::vector<Vertex> vertices;
        std::string* path;
        void serializeString(char* buffer, size_t bufferSize) {
            size_t length = std::min(path->length(), bufferSize - 1);
            strcpy(buffer, path->c_str());
            buffer[length] = '\0';
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
    inline SapphireRenderer::Shader LineShader;
    inline SapphireRenderer::Shader CircleShader;
    inline SapphireRenderer::Shader AnimationShader;
    inline SapphireRenderer::Shader TextureShader;
    const std::vector<Vertex> RectangleVertices = {
        {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
        {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
    };
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Animation{
        public:
            Animation(const std::string& AnimationFile, const std::string& TexturePath);
            void SelectKeyFrame(VertexBuffer& VBO);
            static void Export(const std::vector<KeyFrame*>& MainKeyframes, const std::string& name);
            void SetSelectedAnimation(std::optional<Animation>& CurrentAnimation);
            const SapphireRenderer::Texture& GetTexture() const { return Texture; }
            static std::vector<KeyFramePair> readKeyFramePairsFromBinaryFile(const std::string& filename);
        private:
            SapphireRenderer::Texture Texture;
            double LastRecoredTime = 0.0;
            unsigned int CurrentKeyFrameIdx = 0;
            std::vector<KeyFramePair> KeyFramesData;
    };
    class Shape{
        public:
            SapphireRenderer::Type ShapeType = SapphireRenderer::Null;
            bool& Wireframe() {return m_Wireframe;}
            Shape(const SapphireRenderer::Shader& shader, const std::vector<Vertex>& Vertices,const std::string& path = "");
            ~Shape();
            // That's the function that actually render's a shape
            virtual void Render(const Transform& transform,const glm::vec4& Color,Camera* cam, bool OutLine, const std::function<void(SapphireRenderer::Shader& shader)>& setUpUniforms);
            void Load(const std::string& path, bool flip = false);
            void SetShader(const SapphireRenderer::Shader& shader, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms) {Shader = shader;}
            const glm::vec2 GetTextureDimensions() const;
            SapphireRenderer::VertexBuffer VertexBuffer;
            void SelectAnimation(const std::string& name);
            std::unordered_map<std::string,Animation*> Animations;
            std::unordered_map<std::string,SapphireRenderer::Texture> Textures;
        protected:
            std::optional<Animation> CurrentAnimation = std::nullopt;
            bool HasTexture = false;
            SapphireRenderer::Shader Shader;
            SapphireRenderer::VertexArray VertexArray;
            SapphireRenderer::IndexBuffer IndexBuffer;
            SapphireRenderer::Texture Texture;
            glm::mat4 m_Projection;
            bool m_Wireframe = false;
    };

}

