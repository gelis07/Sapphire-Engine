#pragma once
#include "CollisionDetection.h"
#include "Objects/Transform.h"
#include "AABB.h"
class Object;
constexpr float FixedTimeStep = 1/120.0f;
namespace SapphirePhysics{
    class RigidBody : public Component{
        public:
            static void Run();
            Transform* transform;
            int ShapeType;
            RigidBody();
            RigidBody(const RigidBody& rb);
            void Update();
            static bool IntersectAABBs(AABB a, AABB b);
            void BroadPhase(int Index);
            void NarrowPhase();
            void SetStartingVel(const glm::vec3& startingVel) {StartingVelocity = startingVel;}
            inline static std::vector<std::pair<RigidBody*, RigidBody*>> ContactPairs;
            AABB GetAABB();
            static void OnCollisionRotation(RigidBody* current, RigidBody* obj, CollisionData&& CD);
            void Simulate(int Index);
            static int Impulse(lua_State* L);
            static int RayCast(lua_State* L);
            static int SetVelocity(lua_State* L);
            static int GetVelocity(lua_State* L);
            SapphireEngine::Bool Trigger;
            SapphireEngine::Bool Static;
            SapphireEngine::Bool Rotate;
            SapphireEngine::Float Mass;
            SapphireEngine::Float Restitution;
            SapphireEngine::Float StaticFriction;
            SapphireEngine::Float DynamicFriction;
            float Inertia;
            glm::vec3 Velocity = glm::vec3(0);
            glm::vec3 Accelaration = glm::vec3(0);
            glm::vec3 AngularVelocity = glm::vec3(0);
            glm::vec3 AngularAccelaration = glm::vec3(0);
            std::vector<glm::vec3> Forces;
            std::vector<glm::vec3> Torques;
            inline static std::vector<RigidBody*> Rigibodies;
        private:
            glm::vec3 StartingVelocity = glm::vec3(0);
    };
}