#pragma once
#include "Utilities.hpp"
#include "Renderer.h"

namespace SapphireRenderer{
    class VertexBuffer{
        public:
            VertexBuffer(size_t Size, GLbyte* Data, int Usage);
            VertexBuffer();
            ~VertexBuffer();
            void Bind() const;
            void Unbind() const;
            void AssignData(size_t Size, GLbyte* Data, int usage);
            void SubData(size_t Size, GLbyte* Data);
            const GLuint& GetID() const {return ID;}
        private:
            GLuint ID;
    };
    struct VertexBufferElement{
        unsigned int Count;
        unsigned int Type;
        unsigned char Normalized;
        static unsigned int GetSizeOfType(unsigned int type){
            switch (type)
            {
                case GL_FLOAT : return 4;
                case GL_UNSIGNED_INT : return 4;
                case GL_UNSIGNED_BYTE : return 1;
            }
            return 0;
        }
    };
    class VertexBufferLayout{
        public:
            VertexBufferLayout() : Stride(0) {}
            unsigned int GetStride() const {return Stride;}
            const std::vector<VertexBufferElement>& GetElements() const {return Elements;}

            void Push(unsigned int type, unsigned int count);

        private:
            std::vector<VertexBufferElement> Elements;
            unsigned int Stride = 0;
    };
}