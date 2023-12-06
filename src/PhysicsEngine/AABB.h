#pragma once
#include "Utilities.hpp"


namespace SapphirePhysics
{
    class AABB
    {
        public:
            glm::vec3 Min, Max;
            AABB(const glm::vec3& min, const glm::vec3 max);
    };
}