#include "DebugDraw.h"
#include "Engine/Engine.h"
void SapphireEngine::DrawLines(const glm::mat4& view)
{
    glBindVertexArray(VAO);
    SapphireRenderer::LineShader.Bind();
    const glm::vec2& WindowSize = glm::vec2(Engine::GetCameraObject()->GetTransform()->GetSize());
    glm::mat4 projection = glm::ortho( 0.0f, WindowSize.x, 0.0f, WindowSize.y, -1.0f, 1.0f);
    glm::mat4 mvp = projection * view;
    SapphireRenderer::LineShader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    for (size_t i = 0; i < lines.size(); i++)
    {
        // Use the shader program
        SapphireRenderer::LineShader.SetUniform("u_Color", lines[i].Color);
        float vertices[] = {
            lines[i].Start.x, lines[i].Start.y,
            lines[i].End.x,  lines[i].End.y
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glLineWidth(lines[i].width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    glLineWidth(1.0f);
    lines.clear();
    glBindVertexArray(0);
}

void SapphireEngine::AddLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float width)
{
    lines.push_back({start, end, color, width});
}

void SapphireEngine::Init()
{
    float vertices[] = {
        -0.5f, -0.5f,
         0.5f,  0.5f
    };
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
     // Bind the VAO
    glBindVertexArray(VAO);

    // Copy vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}