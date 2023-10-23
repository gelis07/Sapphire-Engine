#pragma once
#include "Utilities.hpp"
#include "Renderer.h"


namespace SapphireRenderer{
    class IndexBuffer{
        public:
            IndexBuffer(size_t Size, GLbyte* Data, int Usage);
            IndexBuffer();
            void Bind() const;
            void Unbind() const;
            void AssignData(size_t Size, GLbyte* Data, int Usage);
            ~IndexBuffer();
            const GLuint& GetID() const {return ID;}
        private:
            GLuint ID;
    };
}