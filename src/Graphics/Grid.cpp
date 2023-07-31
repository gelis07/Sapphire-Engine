#include "Grid.h"
#include "Graphics/ShaderFunc.h"

void Grid::Init()
{
    GLCall(glGenVertexArrays(1, &GridVA));
    GLCall(glGenBuffers(1, &GridVB));
    GLCall(glGenBuffers(1, &GridIB));

    GLCall(glBindVertexArray(GridVA));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, GridVB));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GridIB));
    float Vertices[] = {
        -SCREEN_WIDTH, -SCREEN_HEIGHT,
        SCREEN_WIDTH, -SCREEN_HEIGHT,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        -SCREEN_WIDTH, SCREEN_HEIGHT
    };
    GLCall(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), Vertices, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2,GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };
    LoadShader(GridShader, "Shaders/Grid.glsl");

    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Indices, GL_STATIC_DRAW));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
}

void Grid::Render(glm::vec3& CameraPos, float CameraZoom)
{
    glm::mat4 proj = glm::ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));    
    glm::mat4 mvp = proj * view * model;

    GLCall(glUseProgram(GridShader));
    GLCall(glBindVertexArray(GridVA));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GridIB));

    GLCall(glUniformMatrix4fv(glGetUniformLocation(GridShader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
    GLCall(glUniform1f(glGetUniformLocation(GridShader, "GridSpacing"), 0.5f));
    GLCall(glUniform1f(glGetUniformLocation(GridShader, "CameraZoom"), CameraZoom));
    GLCall(glUniform2f(glGetUniformLocation(GridShader, "CameraSize"), SCREEN_WIDTH, SCREEN_HEIGHT));
    GLCall(glUniform2f(glGetUniformLocation(GridShader, "CenterPoint"), CameraPos.x, CameraPos.y));
    GLCall(glUniform4f(glGetUniformLocation(GridShader, "u_Color"), 0.2f, 0.2f, 0.2f, 1.0f));
    GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    GLCall(glUseProgram(0));
    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}