#pragma once
#include "CollisionDetection.h"


class Object;

namespace PhysicsEngine{
    class Body{
        public:
            glm::vec3* Position;
            glm::vec3* Rotation;
            glm::vec3* Size;
            int ShapeType;
            Body(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables);
            void Update(const float& DeltaTime);
            bool CollisionDetection(Object* current);
            void OnCollision(Object* current, Object* obj, CollisionData&& CD);
            void OnCollisionRotation(Object* current, Object* obj, CollisionData&& CD);

            SapphireEngine::Bool Trigger;
            SapphireEngine::Bool Static;
            SapphireEngine::Float Mass;
            SapphireEngine::Float Restitution;
            float Inertia;
            glm::vec3 Velocity = glm::vec3(0);
            glm::vec3 Accelaration = glm::vec3(0);
            glm::vec3 AngularVelocity = glm::vec3(0);
            glm::vec3 AngularAccelaration = glm::vec3(0);
            glm::vec3 Force = glm::vec3(0);
    };
}