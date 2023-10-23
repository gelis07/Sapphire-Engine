#pragma once
#include "Utilities.hpp"
#include "Renderer.h"

namespace SapphireRenderer{

    class Shader{
        public:
            Shader(const std::string& Path);
            Shader() {}
            ~Shader();
            void Bind();
            void Unbind();
            void SetUniform(const char* Name, float v1);
            void SetUniform(const char* Name, int v1);
            void SetUniform(const char* Name, const glm::vec2& vector);
            void SetUniform(const char* Name, const glm::vec3& vector);
            void SetUniform(const char* Name, const glm::vec4& vector);
            void SetUniform(const char* Name, int val, int boolean, const float* matrix);
            const GLuint& GetID() const {return ID;}
        private:
            GLuint ID;

    };


    struct ShaderProgramSource
    {
        std::string VertexSource;    
        std::string FragmentSource;    
    };

    //Here I'm using one file for the Fragment and Vertex Shader (thank you The Cherno for the amazing tutorial on shaders)
    static ShaderProgramSource ParseShader(const std::string &filepath)
    {   
        std::ifstream stream(filepath);
        enum class ShaderType
        {
            NONE = -1, VERTEX = 0, FRAGMENT = 1
        };
        std::string line;
        std::stringstream ss[2];
        ShaderType type = ShaderType::NONE;
        while(getline(stream, line))
        {
            if(line.find("#shader") != std::string::npos)
            {
                if(line.find("vertex") != std::string::npos)
                    type = ShaderType::VERTEX;
                else if(line.find("fragment") != std::string::npos)
                    type = ShaderType::FRAGMENT;
            } 
            else
            {
                ss[(int)type] << line << "\n";
            }
        }
        return { ss[0].str(), ss[1].str() };
    }
    static void LoadShader(GLuint &sh, std::string path)
    {
        ShaderProgramSource sp;
        sp = ParseShader(path);
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* src = sp.VertexSource.c_str();
        GLCall(glShaderSource(vertexShader, 1, &src, NULL));
        GLCall(glCompileShader(vertexShader));

        int result;
        GLCall(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result));
        if(result == GL_FALSE)
        {
            int length; 
            GLCall(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(vertexShader, length, &length, message));
            std::cout << message << std::endl;
            GLCall(glDeleteShader(vertexShader));
        }
        
        const char* Fsrc = sp.FragmentSource.c_str();
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        GLCall(glShaderSource(fragmentShader, 1, &Fsrc, NULL));
        GLCall(glCompileShader(fragmentShader));

        GLCall(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result));
        if(result == GL_FALSE)
        {
            int length; 
            GLCall(glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(fragmentShader, length, &length, message));
            std::cout << message << std::endl;
            GLCall(glDeleteShader(fragmentShader));
        }

        GLuint shaderProgram = GLCall(glCreateProgram());
        GLCall(glAttachShader(shaderProgram, fragmentShader));
        GLCall(glAttachShader(shaderProgram, vertexShader));
        GLCall(glLinkProgram(shaderProgram));
        GLCall(glUseProgram(shaderProgram));
        sh = shaderProgram;
        GLCall(glUseProgram(0));
    }
}