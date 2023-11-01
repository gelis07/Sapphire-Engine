#include "VertexArray.h"

SapphireRenderer::VertexArray::VertexArray()
{
    GLCall(glGenVertexArrays(1, &ID));
}

SapphireRenderer::VertexArray::~VertexArray()
{
    // GLCall(glDeleteVertexArrays(1, &ID));
}

void SapphireRenderer::VertexArray::AddBuffer(const VertexBuffer &VBO, const VertexBufferLayout &layout)
{
    Bind();
    VBO.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for (size_t i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.Count,element.Type, element.Normalized, layout.GetStride(), reinterpret_cast<const void*>(offset)));
        offset += element.Count * VertexBufferElement::GetSizeOfType(element.Type);
    }
    Unbind();
    VBO.Unbind();
}

void SapphireRenderer::VertexArray::Bind() const
{
    GLCall(glBindVertexArray(ID));
}

void SapphireRenderer::VertexArray::Unbind() const
{
    GLCall(glBindVertexArray(0));
}
