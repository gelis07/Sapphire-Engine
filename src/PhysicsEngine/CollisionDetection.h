#pragma once
#include "Utilities.hpp"
#include "Scripting/Variables.h"


struct CollisionData{
    glm::vec2 Normal;
    float Depth;
    glm::vec2 ContactPoint1, ContactPoint2;
    int ContactPointCount;
};

class Object;
class RigidBody;
namespace PhysicsEngine{
    class CollisionDetection {
        public:
            static SapphireEngine::Float g; 
            //Collision
            static bool CirclexCircle(std::shared_ptr<Object> obj, Object* current);
            static bool RectanglexRectangle(std::shared_ptr<Object> obj, Object *current, CollisionData& CD);
            static bool CirclexRectangle(std::shared_ptr<Object> obj, Object *current);
            static void FindContactPoint(std::shared_ptr<Object> obj, Object *current, glm::vec2& ContactPoint1, glm::vec2& ContactPoint2, int& ContactPointCount);
        private:
            static glm::vec2 FindArithmeticMean(std::array<glm::vec2, 4>& Vertices);
            static void PointSegmentDistance(glm::vec2 p, glm::vec2 a, glm::vec2 b, float& distanceSquared, glm::vec2& cp);
    };
};


