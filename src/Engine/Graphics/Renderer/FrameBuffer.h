#pragma once
#include "Utilities.hpp"
#include "Texture.h"

namespace SapphireRenderer{
    class FrameBuffer{
        public:
            static void CheckFBO();
            FrameBuffer() {}
            ~FrameBuffer();
            void Init();
            void Bind() const;
            void Unbind() const;
            void RescaleFrameBuffer(float width, float height);
            const GLuint& GetID() const {return ID;}
            const Texture& GetTexture() const {return FBOTexture;}
            Texture& GetTexture() {return FBOTexture;}
        private:
            GLuint ID = 0;
            Texture FBOTexture;
    };

    class MultisampleTextureFrameBuffer{
        public:
            MultisampleTextureFrameBuffer() {}
            ~MultisampleTextureFrameBuffer();
            void Blit(float width, float height);
            void RescaleFrameBuffer(float width, float height);
            void Init();
            void Bind() const;
            void Unbind() const;
            FrameBuffer RendFrameBuffer;
        private:
            GLuint ID = 0;
            GLuint texture;
    };
}