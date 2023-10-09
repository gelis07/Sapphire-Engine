#include "Gizmos.h"
#include "Engine/Engine.h"
#include "Shapes.h"
void SapphireEngine::DrawPointGizmos(glm::vec3 Position, glm::vec4 Color)
{
    unsigned int m_Shader  = 1;
    unsigned int m_VertexBuffer = 1;
    unsigned int m_VertexArray = 1;
    unsigned int m_IndexBuffer = 1;
    m_Shader = Shapes::CircleShader;
    GLCall(glGenVertexArrays(1, &m_VertexArray));
    GLCall(glGenBuffers(1, &m_VertexBuffer));
    GLCall(glGenBuffers(1, &m_IndexBuffer));

    GLCall(glBindVertexArray(m_VertexArray));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2,GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Indices, GL_DYNAMIC_DRAW));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
    std::array<glm::vec2, 4> RectPoints;
    RectPoints[0] = glm::vec2(10, 10);
    RectPoints[1] = glm::vec2(-10, 10);
    RectPoints[2] = glm::vec2(10, -10);
    RectPoints[3] = glm::vec2(-10, -10);

    std::vector<Vertex> vertices ={ {RectPoints[3].x , RectPoints[3].y},
                {RectPoints[2].x, RectPoints[2].y},
                {RectPoints[0].x, RectPoints[0].y},
               {RectPoints[1].x, RectPoints[1].y}};


    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>());
    glm::mat4 m_Projection = glm::ortho(0.0f, WindowSize.x, 0.0f, WindowSize.y, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Position);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), Engine::Get().GetPlay().CameraObject->GetTransform()->Position.value<glm::vec3>());   
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size() * 2, vertices.data()));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));


    GLCall(glUseProgram(m_Shader));
    GLCall(glBindVertexArray(m_VertexArray));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));

    //Here is the standard model view projection matrix
    glm::mat4 mvp = m_Projection * view * model;
    glm::vec2 StartPos(Position.x - 10 + Engine::Get().GetPlay().CameraObject->GetTransform()->Position.value<glm::vec3>().x, Position.y - 10 + Engine::Get().GetPlay().CameraObject->GetTransform()->Position.value<glm::vec3>().y);
    GLCall(glUniformMatrix4fv(glGetUniformLocation(m_Shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
    GLCall(glUniform1f(glGetUniformLocation(m_Shader, "RectWidth"), 20));
    GLCall(glUniform1f(glGetUniformLocation(m_Shader, "RectHeight"), 20));
    GLCall(glUniform2f(glGetUniformLocation(m_Shader, "StartPoint"), StartPos.x, StartPos.y));
    GLCall(glUniform1f(glGetUniformLocation(m_Shader, "CameraZoom"), 1));

    GLCall(glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), Color.r, Color.g, Color.b, Color.a));
    GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    GLCall(glUseProgram(0));
    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLCall(glDeleteVertexArrays(1, &m_VertexArray));
    GLCall(glDeleteBuffers(1, &m_IndexBuffer));
    GLCall(glDeleteBuffers(1, &m_VertexBuffer));
}