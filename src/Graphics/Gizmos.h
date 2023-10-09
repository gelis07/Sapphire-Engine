#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"

namespace SapphireEngine{
    void DrawPointGizmos(glm::vec3 Position, glm::vec4 Color);
}