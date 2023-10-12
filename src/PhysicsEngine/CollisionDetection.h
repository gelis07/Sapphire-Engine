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
            static bool CirclexCircle(std::shared_ptr<Object> obj, Object* current,CollisionData& CD);
            static bool RectanglexRectangle(std::shared_ptr<Object> obj, Object *current, CollisionData& CD);
            static bool CirclexRectangle(Object* obj, Object *current,CollisionData& CD);
            static void FindPolygonContactPoint(std::shared_ptr<Object> obj, Object *current, glm::vec2& ContactPoint1, glm::vec2& ContactPoint2, int& ContactPointCount);
            static glm::vec2 FindPolygonCircleContactPoint(const glm::vec2& CirclePosition, const float& Radius, const glm::vec2& PolygonPosition, const std::array<glm::vec2, 4>& PolygonPoints);
        private:
            static int FindClosestPointOnPolygon(const glm::vec2& Position, std::array<glm::vec2, 4> Points);
            static void ProjectCircle(const glm::vec2& Position, float Radius, glm::vec2 Axis, float& o_Min, float& o_Max);
            static glm::vec2 FindArithmeticMean(std::array<glm::vec2, 4>& Vertices);
            static void PointSegmentDistance(glm::vec2 p, glm::vec2 a, glm::vec2 b, float& o_distanceSquared, glm::vec2& o_cp);
    };
};


