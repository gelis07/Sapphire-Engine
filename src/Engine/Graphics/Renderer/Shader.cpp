#include "Shader.h"

SapphireRenderer::Shader::Shader(const std::string &Path)
{
    SapphireRenderer::LoadShader(ID, Path);
}

SapphireRenderer::Shader::~Shader()
{
    // GLCall(glDeleteProgram(ID));
}

void SapphireRenderer::Shader::Bind()
{
    GLCall(glUseProgram(ID));
}

void SapphireRenderer::Shader::Unbind()
{
    GLCall(glUseProgram(0));
}

void SapphireRenderer::Shader::SetUniform(const char *Name, float v1)
{
    GLCall(glUniform1f(glGetUniformLocation(ID, Name), v1));
}

void SapphireRenderer::Shader::SetUniform(const char *Name, int v1)
{
    glUniform1i(glGetUniformLocation(ID, Name), v1);
}

void SapphireRenderer::Shader::SetUniform(const char *Name, const glm::vec2 &vector)
{
    GLCall(glUniform2f(glGetUniformLocation(ID, Name), vector.x, vector.y));
}

void SapphireRenderer::Shader::SetUniform(const char *Name, const glm::vec3 &vector)
{
    GLCall(glUniform3f(glGetUniformLocation(ID, Name), vector.x, vector.y, vector.z));
}

void SapphireRenderer::Shader::SetUniform(const char *Name, const glm::vec4 &vector)
{
    GLCall(glUniform4f(glGetUniformLocation(ID, Name), vector.x, vector.y, vector.z, vector.w));
}

void SapphireRenderer::Shader::SetUniform(const char *Name, int val, int boolean, const float *matrix)
{
    GLCall(glUniformMatrix4fv(glGetUniformLocation(ID, Name), val,boolean, matrix));
}
