#pragma once
#include "CollisionDetection.h"
#include "Objects/Transform.h"
#include "AABB.h"
class Object;

namespace SapphirePhysics{
    class RigidBody : public Component{
        public:
            Transform* transform;
            int ShapeType;
            RigidBody(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false);
            RigidBody(const RigidBody& rb);
            void Update(const float& DeltaTime);
            static bool IntersectAABBs(AABB a, AABB b);
            static void BroadPhase(Object* current);
            static void NarrowPhase();
            void SetStartingVel(const glm::vec3& startingVel) {StartingVelocity = startingVel;}
            inline static std::vector<std::pair<Object*, Object*>> ContactPairs;
            AABB GetAABB();
            static void OnCollisionRotation(Object* current, Object* obj, CollisionData&& CD);
            void Simulate(Object* current, const float& DeltaTime);
            static int Impulse(lua_State* L);
            static int RayCast(lua_State* L);
            static int SetVelocity(lua_State* L);
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
        private:
            glm::vec3 StartingVelocity = glm::vec3(0);
    };
}