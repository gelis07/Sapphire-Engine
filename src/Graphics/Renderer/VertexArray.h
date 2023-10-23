#pragma once
#include "Utilities.hpp"
#include "VertexBuffer.h"

//Thanks to The Cherno! https://www.youtube.com/watch?v=oD1dvfbyf6A&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=14
namespace SapphireRenderer{
    class VertexArray{
        public:
            VertexArray();
            ~VertexArray();
            void AddBuffer(const VertexBuffer& VBO, const VertexBufferLayout& layout);
            void Bind() const;
            void Unbind() const;
            const GLuint& GetID() const {return ID;}
        private:
            GLuint ID;
    };
}