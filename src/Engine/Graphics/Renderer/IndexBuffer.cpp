#include "IndexBuffer.h"

SapphireRenderer::IndexBuffer::IndexBuffer(size_t Size, GLbyte* Data, int Usage)
{
    GLCall(glGenBuffers(1, &ID));
    Bind();
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, Data, Usage));
    Unbind();
}

SapphireRenderer::IndexBuffer::IndexBuffer()
{
    GLCall(glGenBuffers(1, &ID));
}

void SapphireRenderer::IndexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID));
}

void SapphireRenderer::IndexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void SapphireRenderer::IndexBuffer::AssignData(size_t Size, GLbyte* Data, int Usage)
{
    Bind();
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, Data, Usage));
    Unbind();
}

SapphireRenderer::IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &ID));
}
