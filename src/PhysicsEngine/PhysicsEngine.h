#pragma once
#include "Utilities.hpp"
#include "Scripting/Variables.h"


class Object;
class RigidBody;

class PhysicsEngine {
    public:
        //Collision
        static bool CirclexCircle(std::shared_ptr<Object> obj, Object* current);
        static bool RectanglexRectangle(std::shared_ptr<Object> obj, Object *current);
        static bool CirclexRectangle(std::shared_ptr<Object> obj, Object *current);
        static glm::vec3 Impulse(RigidBody* rb, glm::vec3&& Force);
        static SapphireEngine::Float g;
};


