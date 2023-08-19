#pragma once
#include "Utilities.hpp"

class Object;
class RigidBody;

class PhysicsEngine {
    public:
        //Collision
        static void CirclexCircle(std::shared_ptr<Object> obj, Object* current);
        static void RectanglexRectangle(std::shared_ptr<Object> obj, Object *current);
        static void CirclexRectangle(std::shared_ptr<Object> obj, Object *current);
        static glm::vec3 Impulse(RigidBody* rb, glm::vec3&& Force);
        static inline float g = -50.81;
private:
};


