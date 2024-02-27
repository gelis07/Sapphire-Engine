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
    inline SapphireRenderer::Shader BasicShader("Basic", true);
    inline SapphireRenderer::Shader LineShader("Line", true);
    inline SapphireRenderer::Shader CircleShader("Circle", true);
    inline SapphireRenderer::Shader AnimationShader("Animation", true);
    inline SapphireRenderer::Shader TextureShader("Texture", true);
    inline std::vector<SapphireRenderer::Shader*> shaders;
    const std::vector<Vertex> RectangleVertices = {
        {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
        {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
        {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
        {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
    };
    const std::vector<GLuint> RectangleIndices = {
        0,1,2,
        2,3,0
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
            const std::string& GetAnimFile() {return AnimFile;}
        private:
            std::string AnimFile;
            SapphireRenderer::Texture Texture;
            double LastRecoredTime = 0.0;
            unsigned int CurrentKeyFrameIdx = 0;
            std::vector<KeyFramePair> KeyFramesData;
    };
}

