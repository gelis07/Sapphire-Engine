#include "Grid.h"

Grid::~Grid()
{
    delete VertexArray;
    delete VertexBuffer;
    delete IndexBuffer;
    delete Shader;
}

void Grid::Init()
{
    float Vertices[] = {
        -1, -1,
        1, -1,
        1, 1,
        -1, 1
    };
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };
    VertexArray = new SapphireRenderer::VertexArray();
    VertexBuffer = new SapphireRenderer::VertexBuffer(8 * sizeof(float), (GLbyte*)Vertices, GL_STATIC_DRAW);
    IndexBuffer = new SapphireRenderer::IndexBuffer(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_STATIC_DRAW);
    Shader = new SapphireRenderer::Shader("Shaders/Grid.glsl");
    VertexArray->Bind();
    VertexBuffer->Bind();
    IndexBuffer->Bind();

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    VertexArray->AddBuffer(*VertexBuffer, layout);

    VertexBuffer->Unbind();
    IndexBuffer->Unbind();
    VertexArray->Unbind();
}

void Grid::Render(const glm::vec3& CameraPos, float CameraZoom)
{

    Shader->Bind();
    VertexArray->Bind();
    IndexBuffer->Bind();

    Shader->SetUniform("CameraZoom", CameraZoom);
    Shader->SetUniform("CenterPoint", glm::vec2(CameraPos));
    Shader->SetUniform("u_Color", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

    GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    Shader->Unbind();
    VertexArray->Unbind();
    IndexBuffer->Unbind();
    VertexBuffer->Unbind();
}