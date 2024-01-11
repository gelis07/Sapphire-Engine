#include "DebugDraw.h"
#include "Engine/Engine.h"
void SapphireEngine::DrawDebug(const glm::mat4& view)
{
    glBindVertexArray(VAO);
    SapphireRenderer::LineShader.Bind();
    const glm::vec2& WindowSize = glm::vec2(Engine::GetCameraObject()->GetTransform()->GetSize()) TOPIXELS;
    const glm::vec2& WindowPos = glm::vec2(Engine::GetCameraObject()->GetTransform()->GetPosition());
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
    SapphireRenderer::LineShader.Unbind();
    SapphireRenderer::CircleShader.Bind();
    glBindVertexArray(CircleVAO);

    for (size_t i = 0; i < DebugPoints.size(); i++)
    {
        // Use the shader program
        glm::mat4 Model;
        Model = glm::translate(Model, glm::vec3(DebugPoints[i].point, 0) TOPIXELS);
        Model = glm::scale(Model, glm::vec3(DebugPoints[i].size,DebugPoints[i].size,0) TOPIXELS);
        mvp = projection * view * Model;
        SapphireRenderer::CircleShader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
        glm::vec2 StartPos(DebugPoints[i].point.x - DebugPoints[i].size/2 + WindowPos.x, DebugPoints[i].point.y - DebugPoints[i].size/2 + WindowPos.y);
        SapphireRenderer::CircleShader.SetUniform("u_Color", DebugPoints[i].Color);
        SapphireRenderer::CircleShader.SetUniform("RectWidth", DebugPoints[i].size TOPIXELS);
        SapphireRenderer::CircleShader.SetUniform("RectHeight", DebugPoints[i].size TOPIXELS);
        SapphireRenderer::CircleShader.SetUniform("StartPoint", StartPos TOPIXELS);
        SapphireRenderer::CircleShader.SetUniform("CameraZoom", 1.0f);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
    SapphireRenderer::CircleShader.Unbind();


    glBindVertexArray(0);
}

void SapphireEngine::ClearData()
{
    lines.clear();
    DebugPoints.clear();
}

void SapphireEngine::AddLine(const glm::vec2 &start, const glm::vec2 &end, const glm::vec4 &color, float width)
{
    lines.push_back({start TOPIXELS, end TOPIXELS, color, width});
}

void SapphireEngine::AddPoint(const glm::vec2 &point, const glm::vec4 &color, float size)
{
    DebugPoints.push_back({point, color, size});
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

    glGenBuffers(1, &CircleVBO);
    glGenVertexArrays(1, &CircleVAO);
     // Bind the VAO
    glBindVertexArray(CircleVAO);

    float RectVertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f, 
        -0.5f, 0.5f,
    };

    // Copy vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, CircleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectVertices), RectVertices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    

}
/*
    glBindBuffer(GL_ARRAY_BUFFER, CircleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(SapphireRenderer::RectangleVertices.data()), SapphireRenderer::RectangleVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);*/