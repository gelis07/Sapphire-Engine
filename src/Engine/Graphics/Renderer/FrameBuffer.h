#pragma once
#include "Utilities.hpp"
#include "Texture.h"

namespace SapphireRenderer{
    class FrameBuffer{
        public:
            FrameBuffer() {}
            ~FrameBuffer();
            void Init();
            void Bind() const;
            void Unbind() const;
            void RescaleFrameBuffer(float width, float height);
            const GLuint& GetID() const {return ID;}
            const Texture& GetTexture() const {return FBOTexture;}
        private:
            GLuint ID = 0;
            Texture FBOTexture;
    };
}