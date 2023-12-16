#pragma once
#include "Utilities.hpp"
#include "Graphics/Shapes.h"

namespace SapphireEngine{

    void DrawLines(const glm::mat4& view);
    void AddLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float width);
    void Init();
    inline GLuint VBO, VAO;
    struct line{
        glm::vec2 Start = glm::vec2(0);
        glm::vec2 End = glm::vec2(0);
        glm::vec4 Color = glm::vec4(1);
        float width = 1.0f;
    };
    inline std::vector<line> lines;
}