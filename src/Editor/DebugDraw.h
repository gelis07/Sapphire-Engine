#pragma once
#include "Utilities.hpp"
#include "Graphics/Shapes.h"

namespace SapphireEngine{

    void DrawDebug(const glm::mat4& view);
    void ClearData();
    void AddLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float width);
    void AddPoint(const glm::vec2& point, const glm::vec4& color, float size);
    void Init();
    inline GLuint VBO, VAO;
    inline GLuint CircleVBO, CircleVAO;
    struct line{
        glm::vec2 Start = glm::vec2(0);
        glm::vec2 End = glm::vec2(0);
        glm::vec4 Color = glm::vec4(1);
        float width = 1.0f;
    };
    struct DebugPoint{
        glm::vec2 point = glm::vec2(0);
        glm::vec4 Color = glm::vec4(1);
        float size = 1.0f;
    };
    inline std::vector<line> lines;
    inline std::vector<DebugPoint> DebugPoints;
}