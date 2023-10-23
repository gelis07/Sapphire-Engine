#include "VertexBuffer.h"

SapphireRenderer::VertexBuffer::VertexBuffer(size_t Size, GLbyte* Data, int Usage)
{
    GLCall(glGenBuffers(1, &ID));
    Bind();
    GLCall(glBufferData(GL_ARRAY_BUFFER, Size, Data, Usage));
    Unbind();
}

SapphireRenderer::VertexBuffer::VertexBuffer()
{
    GLCall(glGenBuffers(1, &ID));
}

void SapphireRenderer::VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, ID));
}

void SapphireRenderer::VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void SapphireRenderer::VertexBuffer::AssignData(size_t Size, GLbyte* Data, int Usage)
{
    GLCall(glBufferData(GL_ARRAY_BUFFER, Size, Data, Usage));
}

void SapphireRenderer::VertexBuffer::SubData(size_t Size, GLbyte* Data)
{
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, Size, Data));
}

SapphireRenderer::VertexBuffer::~VertexBuffer()
{
    // GLCall(glDeleteBuffers(1, &ID));
}

void SapphireRenderer::VertexBufferLayout::Push(unsigned int type, unsigned int count)
{
    Elements.push_back({count, type, GL_FALSE});
    Stride += VertexBufferElement::GetSizeOfType(type) * count;
}
