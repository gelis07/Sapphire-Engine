#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <sstream>
#include <iostream>
#include <array>
#include <vector>
#include <unordered_map>
#define SCREEN_WIDTH 960.0f
#define SCREEN_HEIGHT 540.0f
#define EXPORT 0

#define GLCall(x) \
    x; \
    { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            const char* errorString = reinterpret_cast<const char*>(glewGetErrorString(error)); \
            std::cerr << "OpenGL Error " << error << " : (" << #x << "), (" << __FILE__ << ":" << __LINE__ << "): " << errorString << std::endl; \
        } \
    }  

namespace Utilities
{ 
    enum LogType{
        Info=0,Warning=1,Error=2
    };
    inline std::vector<std::pair<std::string, Utilities::LogType>> Logs;

    static float ClampFunc(const float min, const float max, float &value){
        // value = std::min(std::max(min,value) , max);
        return std::min(std::max(min,value) , max);
    }
    static void Log(std::string&& log, Utilities::LogType&& type){
        std::pair<std::string, Utilities::LogType> NewLog;
        NewLog.first = log;
        NewLog.second = type;
        Utilities::Logs.push_back(NewLog);
    }
    static float LengthVec(glm::vec2 vec){
        return sqrt(pow((vec.x), 2) + pow((vec.y), 2));
    }
    static std::string Replace(std::string s,char c1, char c2)
    {
        int l = s.length();
    
        for (int i = 0; i < l; i++)
        {
            if (s[i] == c1)
                s[i] = c2;
    
            else if (s[i] == c2)
                s[i] = c1;
        }
        return s;
    }
    static glm::vec4 LoadIconFromAtlas(glm::vec2 coord, glm::vec2 size, glm::vec2 AtlasSize)
    {
        return glm::vec4(coord.x / AtlasSize.x, coord.y / AtlasSize.y, (coord.x + size.x) / AtlasSize.x, (coord.y + size.y) / AtlasSize.y);
    }
    static float DegToRad(float deg){
        float pi = 3.14159265359f;
        return deg * (pi/180);
    }
    static glm::vec2 VectorSum(std::vector<glm::vec2> vectors){
        glm::vec2 sum;
        for(glm::vec2 vec : vectors){
            sum += vec;
        }
        return sum;
    }
} 
