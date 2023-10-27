#pragma once
#include "CollisionDetection.h"
#include "Objects/Transform.h"

class Object;

namespace PhysicsEngine{
    class RigidBody : public Component{
        public:
            Transform* transform;
            int ShapeType;
            RigidBody(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,bool LuaComp = false);
            void Update(const float& DeltaTime);
            bool CollisionDetection(Object* current);
            void OnCollision(Object* current, Object* obj, CollisionData&& CD);
            void OnCollisionRotation(Object* current, Object* obj, CollisionData&& CD);
            void Simulate(Object *current, const float& DeltaTime);
            static int Impulse(lua_State* L);
            SapphireEngine::Bool Trigger;
            SapphireEngine::Bool Static;
            SapphireEngine::Float Mass;
            SapphireEngine::Float Restitution;
            SapphireEngine::Float StaticFriction;
            SapphireEngine::Float DynamicFriction;

            float Inertia;
            glm::vec3 Velocity = glm::vec3(0);
            glm::vec3 Accelaration = glm::vec3(0);
            glm::vec3 AngularVelocity = glm::vec3(0);
            glm::vec3 AngularAccelaration = glm::vec3(0);
            glm::vec3 Force = glm::vec3(0);
    };
}