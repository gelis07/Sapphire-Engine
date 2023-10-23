#include "Grid.h"
#include "Graphics/ShaderFunc.h"

void Grid::Init()
{
    float Vertices[] = {
        -960, -560,
        960, -560,
        960, 560,
        -960, 560
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

void Grid::Render(glm::vec3& CameraPos, float CameraZoom)
{
    glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 560.0f, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));    
    glm::mat4 mvp = proj * view * model;

    Shader->Bind();
    VertexArray->Bind();
    IndexBuffer->Bind();

    Shader->SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    Shader->SetUniform("GridSpacing", 0.5f);
    Shader->SetUniform("CameraZoom", CameraZoom);
    Shader->SetUniform("CameraSize", glm::vec2(960, 560));
    Shader->SetUniform("CenterPoint", glm::vec2(CameraPos));
    Shader->SetUniform("u_Color", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

    GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    Shader->Unbind();
    VertexArray->Unbind();
    IndexBuffer->Unbind();
    VertexBuffer->Unbind();
}