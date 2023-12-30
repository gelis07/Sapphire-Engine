#pragma once
#include "pch.h"
#define GLCall(x) \
    x; \
    { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            const char* errorString = reinterpret_cast<const char*>(glewGetErrorString(error)); \
            std::cerr << "OpenGL Error " << error << " : (" << #x << "), (" << __FILE__ << ":" << __LINE__ << "): " << errorString << std::endl; \
        } \
    }  

#define Stopwatch(x) \
    {\
        float StartingTime = glfwGetTime(); \
        x;\
        std::stringstream ss; \
        ss << "(" << __FILE__ << ":" << __LINE__ << ")"; \
        if(SapphireEngine::stats.find(ss.str()) == SapphireEngine::stats.end()){ \
            SapphireEngine::stats[ss.str()] = glfwGetTime() - StartingTime; \
        } else{ \
            SapphireEngine::stats[ss.str()] = (SapphireEngine::stats[ss.str()] + (glfwGetTime() - StartingTime)) / SapphireEngine::FrameCount; \
        }\
    }



class ImGuiLogger {
public:
    ImGuiLogger() {}

    const std::ostringstream& GetContent() {return logBuffer;}
    const uint32_t& Size() {return LogCount;}
    // Log a message
    template <typename T>
    ImGuiLogger& operator<<(const T& value) {
        logBuffer << value;
        coutStream << value; // Optionally, you can still send the message to std::cout
        return *this;
    }

    // Log manipulators (e.g., std::endl)
    ImGuiLogger& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
        logBuffer << manipulator;
        coutStream << manipulator;
        return *this;
    }
    void Clear(){
        logBuffer.str(std::string());
        LogCount = 0;
    }

    uint32_t LogCount = 0;
private:
    std::ostringstream logBuffer; // Buffer for ImGui output
    std::ostream coutStream{nullptr}; // Original std::cout stream (optional)
};

namespace SapphireEngine
{ 
    enum LogType{
        Info=0,Warning=1,Error=2
    };
    // inline std::vector<std::pair<std::string, SapphireEngine::LogType>> Logs;
    inline std::unordered_map<std::string, double> stats;
    inline int FrameCount = 0;
    inline double FrameRate = 0;
    inline ImGuiLogger Logs;
    static float Clamp(const float min, const float max, float &value){
        // value = std::min(std::max(min,value) , max);
        return std::min(std::max(min,value) , max);
    }
    static glm::vec4 RandomColor(){
        std::random_device rd;
        std::mt19937 gen(rd());  // Mersenne Twister PRNG
        // Define a distribution for floating-point numbers between 0.0 and 1.0
        std::uniform_real_distribution<float> dist(0.0, 1.0);
        return glm::vec4(dist(gen), dist(gen), dist(gen), 1);
    }
    static void Log(const std::string& log, SapphireEngine::LogType&& type){
        Logs << log << '\n';
        Logs.LogCount++;
    }
    static void Log(const std::string& log){
        Logs << log << '\n';
        Logs.LogCount++;
    }
    static float LengthVec(glm::vec2 vec){
        return sqrt(pow((vec.x), 2) + pow((vec.y), 2));
    }
    static float LengthVec(glm::vec2 vec1, glm::vec2 vec2){
        return sqrt(pow((vec1.x - vec2.x), 2) + pow((vec1.y - vec2.y), 2));
    }
    static int RandomNumber(int start, int end){
        std::random_device rd;
        std::mt19937 rng(rd());

        std::uniform_int_distribution<int> distribution(start, end);

        return distribution(rng);
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
        constexpr float pi = 3.14159265359f;
        return deg * (pi/180);
    }
    static glm::vec3 VectorSum(std::vector<glm::vec3> vectors){
        glm::vec3 sum(0);
        for(glm::vec3 vec : vectors){
            sum += vec;
        }
        return sum;
    }
} 
