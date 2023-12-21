#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"
#include <thread>
#include <execution>
#include <glm/gtx/norm.hpp>
#include "Editor/DebugDraw.h"
#define ITERATIONS 4
#define VECTOR_THRESHOLD 1e-12

SapphirePhysics::RigidBody::RigidBody(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp) : Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),
    Component(std::move(File), std::move(ArgName), ArgId,LuaComp)
{
    Trigger.Get() = false;
    Static.Get() = false;
    Restitution.Get() = 0.0f;
    StaticFriction.Get() = 0.6f;
    DynamicFriction.Get() = 0.4f;
    Restitution.Min = 0.0f;
    Restitution.Max = 1.0f;
    DynamicFriction.Min = 0.0f;
    DynamicFriction.Max = 1.0f;
    StaticFriction.Min = 0.0f;
    StaticFriction.Max = 1.0f;
    StaticFriction.SliderSpeed = 0.01f;
    DynamicFriction.SliderSpeed = 0.01f;
    Restitution.SliderSpeed = 0.01f;
    Mass.Get() = 1.0f;
    Mass.Min = 0.001f;
    Functions["Impulse"] = Impulse;
    Functions["SetVelocity"] = SetVelocity;
}

SapphirePhysics::RigidBody::RigidBody(const RigidBody &rb): Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),
Component(std::move(""), std::move("Renderer"), 0, false)
{
    Trigger.Get() = rb.Trigger.Get();
    Static.Get() = rb.Static.Get();
    Restitution.Get() = rb.Restitution.Get();
    StaticFriction.Get() = rb.StaticFriction.Get();
    DynamicFriction.Get() = rb.DynamicFriction.Get();
    Restitution.Min = 0.0f;
    Restitution.Max = 1.0f;
    DynamicFriction.Min = 0.0f;
    DynamicFriction.Max = 1.0f;
    StaticFriction.Min = 0.0f;
    StaticFriction.Max = 1.0f;
    Restitution.SliderSpeed = 0.01f;
    StaticFriction.SliderSpeed = 0.01f;
    DynamicFriction.SliderSpeed = 0.01f;
    Mass.Get() = rb.Mass.Get();
    Mass.Min = 0.001f;
    Functions["Impulse"] = Impulse;
    Functions["SetVelocity"] = SetVelocity;
}

void SapphirePhysics::RigidBody::Update(const float &DeltaTime)
{
    if(Static.Get()) return;
    Force = glm::vec3(0, SapphirePhysics::CollisionDetection::g.Get(), 0) * Mass.Get();
    Accelaration = (Force/Mass.Get());
    Velocity += StartingVelocity + Accelaration * DeltaTime;

    transform->Move(Velocity * DeltaTime);
    transform->Rotate(AngularVelocity.z * DeltaTime);
    StartingVelocity = glm::vec3(0);
}
struct StackObjectDeleter {
    void operator()(Object* /* object */) const {
        // Custom deleter, no need to delete stack-allocated objects7
    }
};

bool SapphirePhysics::RigidBody::IntersectAABBs(AABB a, AABB b)
{
    return !(a.Max.x <= b.Min.x || b.Max.x <= a.Min.x
        || a.Max.y <= b.Min.y || b.Max.y <= a.Min.y);
}

void SapphirePhysics::RigidBody::BroadPhase(Object* current)
{
    if(current->GetRb()->ShapeType == SapphireRenderer::RectangleT){
        std::for_each(std::execution::par,Engine::GetActiveScene().Objects.begin(), Engine::GetActiveScene().Objects.end(), [current](auto&& object) {
            if(object.Name == "MainCamera" || &object == current || object.GetRb() == nullptr) return;
            if(!IntersectAABBs(object.GetRb()->GetAABB(), current->GetRb()->GetAABB())) {
                return;
            };
            if(object.GetRb()->Static.Get() && current->GetRb()->Static.Get()) return;
            ContactPairs.push_back(std::pair<Object*, Object*>(current, &object));
        });
    }
}

void SapphirePhysics::RigidBody::NarrowPhase()
{
    //! FOR SOME REASON IT WORKS WITH SHARED POITNERS ONLY I HAVE NO IDEA WHY.

    std::for_each(std::execution::par,ContactPairs.begin(), ContactPairs.end(), [](auto&& pair) {
        CollisionData CD;
        std::shared_ptr<Object> sharedObject(pair.second,StackObjectDeleter{});
        if(SapphirePhysics::CollisionDetection::RectanglexRectangle(sharedObject, pair.first,CD)){
            // pair.first->OnCollision(pair.second);
            // pair.second->OnCollision(pair.first);
            OnCollisionRotation(pair.first, pair.second, std::move(CD));
        }
    });
}

SapphirePhysics::AABB SapphirePhysics::RigidBody::GetAABB()
{
    glm::vec3 min,max;
    if(ShapeType == SapphireRenderer::RectangleT)
    {
        for (auto &&point : transform->GetPoints())
        {
            if(point.x < min.x) min.x = point.x;    
            if(point.x > max.x) max.x = point.x;    
            if(point.y < min.y) min.y = point.y;    
            if(point.y > max.y) max.y = point.y;    
        }
        
    }else if(ShapeType == SapphireRenderer::CircleT){
        min.x = transform->GetPosition().x - transform->GetSize().x;
        min.y = transform->GetPosition().y - transform->GetSize().y;
        max.x = transform->GetPosition().x + transform->GetSize().x;
        max.y = transform->GetPosition().y + transform->GetSize().y;
    }
    return AABB(min,max);
}
float CrossProduct(glm::vec2 v1, glm::vec2 v2){
    return v1.x * v2.y - v1.y * v2.x;
}
const float ImpulseThreshold = 10.0f;
void SapphirePhysics::RigidBody::OnCollisionRotation(Object* ObodyA, Object* ObodyB, CollisionData &&CD)
{
    RigidBody* bodyA = ObodyA->GetRb().get();
    RigidBody* bodyB = ObodyB->GetRb().get();
    const glm::vec3& bodyAPos = bodyA->transform->GetPosition();
    const glm::vec3& bodyASize = bodyA->transform->GetSize();
    const glm::vec3& bodyBPos = bodyB->transform->GetPosition();
    const glm::vec3& bodyBSize = bodyB->transform->GetSize();
    float bodyAInvMass = 1.0f / bodyA->Mass.Get();
    float bodyBInvMass = 1.0f / bodyB->Mass.Get();
    float bodyAInvInertia = 1.0f / ((1.0f / 12.0f) * bodyA->Mass.Get() * (bodyASize.x * bodyASize.x + bodyASize.y * bodyASize.y));
    float bodyBInvInertia = 1.0f / ((1.0f / 12.0f) * bodyB->Mass.Get() * (bodyBSize.x * bodyBSize.x + bodyBSize.y * bodyBSize.y));
    if(bodyA->Static.Get()){
        bodyAInvMass = 0.0f;
        bodyAInvInertia = 0.0f;
    }
    if(bodyB->Static.Get()){
        bodyBInvMass = 0.0f;
        bodyBInvInertia = 0.0f;
    }
    //Not realistic, but good enough for a video game.
    float StaticFriction = (bodyA->StaticFriction.Get() + bodyB->StaticFriction.Get()) * 0.5f;
    float DynamicFriction = (bodyA->DynamicFriction.Get() + bodyB->DynamicFriction.Get()) * 0.5f;
    glm::vec2 normal = CD.Normal;
    glm::vec2 contact1 = CD.ContactPoint1;
    glm::vec2 contact2 = CD.ContactPoint2;
    int contactCount = CD.ContactPointCount;

    float e = std::min(bodyA->Restitution.Get(), bodyB->Restitution.Get());
    std::array<glm::vec2, 2> contactList {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec2, 2> impulseList = {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec2, 2> FrictionImpulseList = {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec2, 2> raList = {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec2, 2> rbList = {glm::vec2(0),glm::vec2(0)};
    std::array<float, 2> JList;
    contactList[0] = contact1;
    contactList[1] = contact2;

    for (int i = 0; i < contactCount; i++)
    {
        glm::vec2 ra = contactList[i] - glm::vec2(bodyAPos);
        glm::vec2 rb = contactList[i] - glm::vec2(bodyBPos);

        raList[i] = ra;
        rbList[i] = rb;

        glm::vec2 raPerp = glm::vec2(-ra.y, ra.x);
        glm::vec2 rbPerp = glm::vec2(-rb.y, rb.x);
        // raPerp = glm::normalize(raPerp);
        // rbPerp = glm::normalize(rbPerp);
        glm::vec2 angularLinearVelocityA = raPerp * bodyA->AngularVelocity.z;
        glm::vec2 angularLinearVelocityB = rbPerp * bodyB->AngularVelocity.z;

        glm::vec2 relativeVelocity = 
            (glm::vec2(bodyB->Velocity) + angularLinearVelocityB) - 
            (glm::vec2(bodyA->Velocity) + angularLinearVelocityA);

        float contactVelocityMag = glm::dot(relativeVelocity, normal);

        if (contactVelocityMag > 0.0f)
        {
            continue;
        }

        float raPerpDotN = glm::dot(raPerp, normal);
        float rbPerpDotN = glm::dot(rbPerp, normal);

        float denom = bodyAInvMass + bodyBInvMass + 
            (raPerpDotN * raPerpDotN) * bodyAInvInertia + 
            (rbPerpDotN * rbPerpDotN) * bodyBInvInertia;

        float j = -(1.0f + e) * contactVelocityMag;
        if(denom != 0) j /= denom;
        if(contactCount != 0) j /= (float)contactCount;
        JList[i] = j;
        glm::vec2 impulse = j * normal;
        impulseList[i] = impulse;
    }
    for(int i = 0; i < contactCount; i++)
    {
        glm::vec2 impulse = impulseList[i];
        glm::vec2 ra = raList[i];
        glm::vec2 rb = rbList[i];
        bodyA->Velocity += glm::vec3(-impulse * bodyAInvMass,0);
        bodyA->AngularVelocity.z += -CrossProduct(ra, impulse) * bodyAInvInertia;
        SapphireEngine::AddLine(glm::vec2(bodyAPos), glm::vec2(bodyAPos) + glm::vec2(-impulse * bodyAInvMass), glm::vec4(0.2f, 0.5f, 0.1f, 1.0f), 5.0f);
        bodyB->Velocity += glm::vec3(impulse * bodyBInvMass,0);
        bodyB->AngularVelocity.z += CrossProduct(rb, impulse) * bodyBInvInertia;
        SapphireEngine::AddLine(glm::vec2(bodyBPos), glm::vec2(bodyBPos)+glm::vec2(impulse * bodyAInvMass), glm::vec4(0.2f, 0.5f, 0.1f, 1.0f), 5.0f);
    }

    for (int i = 0; i < contactCount; i++)
    {
        glm::vec2 ra = contactList[i] - glm::vec2(bodyAPos);
        glm::vec2 rb = contactList[i] - glm::vec2(bodyBPos);

        raList[i] = ra;
        rbList[i] = rb;

        glm::vec2 raPerp = glm::vec2(-ra.y, ra.x);
        glm::vec2 rbPerp = glm::vec2(-rb.y, rb.x);
        raPerp = glm::normalize(raPerp);
        rbPerp = glm::normalize(rbPerp);
        glm::vec2 angularLinearVelocityA = raPerp * bodyA->AngularVelocity.z;
        glm::vec2 angularLinearVelocityB = rbPerp * bodyB->AngularVelocity.z;

        glm::vec2 relativeVelocity = 
            (glm::vec2(bodyB->Velocity) + angularLinearVelocityB) - 
            (glm::vec2(bodyA->Velocity) + angularLinearVelocityA);

        glm::vec2 tangent = relativeVelocity - glm::dot(relativeVelocity, normal) * normal;
        if(tangent == glm::vec2(0)){
            continue;
        }else{
            float squaredLength = glm::length2(tangent);
            float length = 1;
            if (squaredLength > VECTOR_THRESHOLD) {
                length = glm::sqrt(squaredLength);
                tangent = glm::vec2(tangent.x/length, tangent.y/length);
            } else {
                //Vector is very close to 0, consider it as 0.
                continue;
            }
            if(std::isnan(tangent.x) || std::isnan(tangent.y)){
                //Vector was close to 0 and the length came out as 0 and then normalized which caused the tangent to be NaN.
                continue;
            }
        }
        float raPerpDotT = glm::dot(raPerp, tangent);
        float rbPerpDotT = glm::dot(rbPerp, tangent);

        float denom = bodyAInvMass + bodyBInvMass + 
            (raPerpDotT * raPerpDotT) * bodyAInvInertia + 
            (rbPerpDotT * rbPerpDotT) * bodyBInvInertia;

        float jt = -glm::dot(relativeVelocity, tangent);
        if(denom != 0) jt /= denom;
        if(contactCount != 0) jt /= (float)contactCount;
        glm::vec2 FrictionImpulse;
        if(abs(jt) <= JList[i] * StaticFriction){
            FrictionImpulse = jt * tangent;
        }else{
            FrictionImpulse = -JList[i] * tangent * DynamicFriction;
        }
        FrictionImpulseList[i] = FrictionImpulse;
    }
    for(int i = 0; i < contactCount; i++)
    {
        glm::vec2 impulse = FrictionImpulseList[i];
        glm::vec2 ra = raList[i];
        glm::vec2 rb = rbList[i];

        bodyA->Velocity += glm::vec3(-impulse * bodyAInvMass,0);
        bodyA->AngularVelocity.z += -CrossProduct(glm::normalize(ra), impulse) * bodyAInvInertia;
        SapphireEngine::AddLine(glm::vec2(bodyAPos), glm::vec2(bodyAPos) + glm::vec2(-impulse * bodyAInvMass), glm::vec4(0.7f, 0.3f, 0.8f, 1.0f), 5.0f);
        bodyB->Velocity += glm::vec3(impulse * bodyBInvMass,0);
        bodyB->AngularVelocity.z += CrossProduct(glm::normalize(rb), impulse) * bodyBInvInertia;
        SapphireEngine::AddLine(glm::vec2(bodyBPos), glm::vec2(bodyBPos)+glm::vec2(impulse * bodyAInvMass), glm::vec4(0.7f, 0.3f, 0.8f, 1.0f), 5.0f);
    }
    if(bodyB->Static.Get())
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth,0));
    else if(bodyA->Static.Get())
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth,0));
    else{
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth / 2.0f,0));
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth / 2.0f,0));
    } 
}
void SapphirePhysics::RigidBody::Simulate(Object* current, const float& DeltaTime) {
    for (size_t i = 0; i < ITERATIONS; i++)
    {
        ContactPairs.clear();
        Update(DeltaTime / ITERATIONS);
        BroadPhase(current);
        NarrowPhase();
    } 
}

int SapphirePhysics::RigidBody::Impulse(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    glm::vec3 Force(x,y,z);
    float& mass = rb->Mass.Get();
    glm::vec3 weight = glm::vec3(0,SapphirePhysics::CollisionDetection::g.Get() * mass,0);
    glm::vec3 Fnet = Force - weight;
    float DeltaTime = 0.2f;
    rb->Velocity = glm::vec3(Fnet * DeltaTime) / mass;
    return 0;
}

int SapphirePhysics::RigidBody::SetVelocity(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    glm::vec3 velocity(x,y,0);
    rb->SetStartingVel(velocity);
    return 0;
}
