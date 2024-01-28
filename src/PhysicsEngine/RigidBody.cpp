#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"
#include <thread>
#include <execution>
#include <glm/gtx/norm.hpp>
#include "Editor/DebugDraw.h"
#define ITERATIONS 1
#define VECTOR_THRESHOLD 1e-12

void SapphirePhysics::RigidBody::Run()
{
    for (size_t i = 0; i < Rigibodies.size(); i++)
    {
        Rigibodies[i]->Simulate(i);
    }
}

SapphirePhysics::RigidBody::RigidBody(int st) : Trigger("Trigger", Variables),
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("StaticFriction", Variables), DynamicFriction("DynamicFriction", Variables), Rotate("Rotate", Variables),
Component("Rigidbody")
{
    ShapeType = st;
    Trigger.Get() = false;
    Static.Get() = false;
    Rotate.Get() = true;
    Restitution.Get() = 0.2f;
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
    Functions["GetVelocity"] = GetVelocity;
    Functions["Raycast"] = RayCast;
}

SapphirePhysics::RigidBody::RigidBody(const RigidBody &rb): Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),Rotate("Rotate", Variables),
Component("Rigidbody")
{
    Trigger.Get() = rb.Trigger.Get();
    Static.Get() = rb.Static.Get();
    Rotate.Get() = rb.Rotate.Get();
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
    Functions["GetVelocity"] = GetVelocity;
    Functions["Raycast"] = RayCast;
}

SapphirePhysics::RigidBody::~RigidBody()
{
    for (size_t i = 0; i < Rigibodies.size(); i++)
    {
        if(this == Rigibodies[i]){
            Rigibodies.erase(Rigibodies.begin() + i);
        }
    }
}

void SapphirePhysics::RigidBody::Update()
{
    if(Static.Get()) return;
    Forces.push_back(glm::vec3(0, SapphirePhysics::CollisionDetection::g.Get(), 0) * Mass.Get());
    Accelaration = (SapphireEngine::VectorSum(Forces)/Mass.Get());
    AngularAccelaration = SapphireEngine::VectorSum(Torques) / (((1.0f / 12.0f) * Mass.Get() * (transform->GetSize().x * transform->GetSize().x + transform->GetSize().y * transform->GetSize().y)));
    Velocity += Accelaration * FixedTimeStep;
    AngularVelocity.z += AngularAccelaration.z * FixedTimeStep;
    transform->Move(Velocity * FixedTimeStep);
    if(Rotate.Get())
        transform->Rotate(AngularVelocity.z * FixedTimeStep);
    StartingVelocity = glm::vec3(0);
    Forces.clear();
    Torques.clear();
}
bool SapphirePhysics::RigidBody::IntersectAABBs(AABB a, AABB b)
{
    return !(a.Max.x <= b.Min.x || b.Max.x <= a.Min.x
        || a.Max.y <= b.Min.y || b.Max.y <= a.Min.y);
}

void SapphirePhysics::RigidBody::BroadPhase(int Index)
{
    std::for_each(std::execution::par,(Rigibodies.begin() + Index), Rigibodies.end(), [this](auto&& object) {
        if(object == this) return;
        if(!IntersectAABBs(object->GetAABB(), this->GetAABB())) {
            return;
        };
        if(object->Static.Get() && this->Static.Get()) return;
        ContactPairs.push_back(std::pair<RigidBody*, RigidBody*>(this, object));
    });
}

void SapphirePhysics::RigidBody::NarrowPhase()
{
    for (auto &&pair : ContactPairs)
    {
        std::stringstream ss;
        CollisionData CD;
        if(SapphirePhysics::CollisionDetection::RectanglexRectangle(pair.second, pair.first,CD)){
            // if(!(pair.first->CalledOnCollision)){
            //     pair.first->OnCollision(pair.second);
            //     std::cout << "called first" << '\n';
            //     pair.first->CalledOnCollision = true;
            // }
            // if(!(pair.second->CalledOnCollision)){
            //     pair.second->OnCollision(pair.first);
            //     std::cout << "called second" << '\n';
            //     pair.second->CalledOnCollision = true;
            // }
            OnCollisionRotation(pair.first, pair.second, std::move(CD));  
        }
    }
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
//Thanks to the book "Physics for Game Developers" from David M. Bourg and Bryan Bywalec.
void SapphirePhysics::RigidBody::OnCollisionRotation(RigidBody* bodyA, RigidBody* bodyB, CollisionData &&CD)
{
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

    std::array<glm::vec2, 2> contactList {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec3, 2> FrictionImpulseList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> raList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> rbList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> VpA = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> VpB = {glm::vec3(0),glm::vec3(0)};
    std::array<float, 2> JList;
    contactList[0] = CD.ContactPoint1;
    contactList[1] = CD.ContactPoint2;
    //coefficient of restitution.
    float e = std::min(bodyA->Restitution.Get(), bodyB->Restitution.Get());
    //coefficient of friction.
    float cf = (bodyA->StaticFriction.Get() + bodyB->StaticFriction.Get()) * 0.5f;

    glm::vec3 n = glm::vec3(CD.Normal,0);
    for (size_t i = 0; i < CD.ContactPointCount; i++)
    {
        glm::vec2 Point = contactList[i];
        glm::vec3 r1 = glm::vec3(Point - glm::vec2(bodyAPos), 0);
        glm::vec3 r2 = glm::vec3(Point - glm::vec2(bodyBPos), 0);
        glm::vec3 Vp1 = bodyA->Velocity + glm::cross(bodyA->AngularVelocity, r1);
        glm::vec3 Vp2 = bodyB->Velocity + glm::cross(bodyB->AngularVelocity, r2);
        glm::vec3 RelativeVelocity = Vp1 - Vp2;
        float J = -glm::dot(RelativeVelocity, n) * (e + 1) 
        / (bodyAInvMass + bodyBInvMass 
        + glm::dot(n, glm::cross(glm::cross(r1, n) * bodyAInvMass, r1))
        + glm::dot(n, glm::cross(glm::cross(r2, n) * bodyBInvInertia, r2)));
        JList[i] = J;
        raList[i] = r1;
        rbList[i] = r2;
        VpA[i] = Vp1;
        VpB[i] = Vp2;

    }
    glm::vec3 t = glm::normalize(bodyA->Velocity - bodyB->Velocity);

    if(bodyB->Static.Get()){
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth,0));
        bodyA->Forces.push_back(JList[0]*n/FixedTimeStep);
        bodyA->Forces.push_back(cf * JList[0]*t /FixedTimeStep);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            bodyA->Torques.push_back(glm::cross(raList[i], (JList[i] * n) + (cf * JList[i] * VpA[i])));
        }
        
    }
    else if(bodyA->Static.Get()){
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth,0));
        bodyB->Forces.push_back(-JList[0]*n/FixedTimeStep);
        bodyB->Forces.push_back(cf * -JList[0]*t /FixedTimeStep);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            bodyB->Torques.push_back(glm::cross(rbList[i], -JList[i] * n + (cf * JList[i] * VpB[i])));
        }
    }
    else{
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth / 2.0f,0));
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth / 2.0f,0));

        bodyA->Forces.push_back(JList[0]*n/FixedTimeStep);
        bodyB->Forces.push_back(-JList[0]*n/FixedTimeStep);

        bodyB->Forces.push_back(cf * -JList[0]*t /FixedTimeStep);
        bodyA->Forces.push_back(cf * JList[0]*t /FixedTimeStep);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            bodyA->Torques.push_back(glm::cross(raList[i], (JList[i] * n) + (cf * JList[i] * VpA[i])));
            bodyB->Torques.push_back(glm::cross(rbList[i], -JList[i] * n + (cf * JList[i] * VpB[i])));
        }
        
    } 
}
void SapphirePhysics::RigidBody::Simulate(int Index) {
    for (auto &object : Engine::GetActiveScene().Objects)
    {
        object.CalledOnCollision = false;
    }
    for (size_t i = 1; i <= ITERATIONS; i++)
    {
        ContactPairs.clear();
        Update();
        BroadPhase(Index);
        NarrowPhase();
    }
}

int SapphirePhysics::RigidBody::Impulse(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, -1, "x");
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "y");
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    glm::vec3 Force(x,y,0);
    rb->Forces.push_back(Force/FixedTimeStep);
    return 0;
}

int SapphirePhysics::RigidBody::RayCast(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    std::string tag;
    if(lua_gettop(L) == 3){
        tag = "";
    }else{
        tag = std::string(lua_tostring(L, 4));
    }
    glm::vec2 A = rb->transform->GetWorldPositon();
    glm::vec2 B(x,y);
    glm::vec2 AB = B - A;
    float tangent = (B.y - A.y) / (B.x - A.x);
    float offset = A.y - tangent * A.x;
    SapphireEngine::AddLine(A,B,glm::vec4(0,0,1,1), 5.0f);
    std::optional<glm::vec2> FinalPoint = std::nullopt;
    ObjectRef obj(null_ref);
    for (auto &object : Engine::GetActiveScene().Objects)
    {
        if(object.GetComponent<SapphirePhysics::RigidBody>().get() == rb) continue;
        if(tag != "" && object.Tag != tag) continue;
        for (size_t i = 0; i < object.GetComponent<Transform>()->GetPoints().size(); i++)
        {
            glm::vec2 C = object.GetComponent<Transform>()->GetPoints()[i];
            glm::vec2 D = object.GetComponent<Transform>()->GetPoints()[(i + 1) % object.GetComponent<Transform>()->GetPoints().size()];
            SapphireEngine::AddLine(C, D, glm::vec4(1,0,0,1), 5.0f);
            glm::vec2 CD = D - C;
            float tangent1 = (D.y - C.y) / (D.x - C.x);
            float offset1 = C.y - tangent1 * C.x;
            //The line is vertical so the function doesn't exist.
            if(tangent1 == INFINITY || tangent1 == -INFINITY){

                float Kx = C.x;
                glm::vec2 CollisionPoint(C.x, tangent*C.x+offset);
                glm::vec2 CCollisionPoint = CollisionPoint - C;
                glm::vec2 ACollisionPoint = CollisionPoint - A;
                bool Collision1 = glm::length(ACollisionPoint) <= glm::length(AB) && glm::dot(ACollisionPoint, AB) > 0;
                bool Collision2 = glm::length(CCollisionPoint) <= glm::length(CD) && glm::dot(CCollisionPoint, CD) > 0;
                //TODO: Abstact that
                if(Collision1 && Collision2){
                    if(FinalPoint == std::nullopt)
                    {
                        FinalPoint = CollisionPoint;
                        obj = object.GetRef();
                    }
                    else{
                        if(glm::length(A - FinalPoint.value()) > glm::length(A - CollisionPoint)){
                            FinalPoint = CollisionPoint;
                            obj = object.GetRef();
                        }
                    }
                }
                continue;
            }
            float Kx = (offset - offset1)/(tangent1 - tangent);
            glm::vec2 CollisionPoint(Kx, tangent1*Kx+offset1);
            glm::vec2 CCollisionPoint = CollisionPoint - C;
            glm::vec2 ACollisionPoint = CollisionPoint - A;
            bool Collision1 = glm::length(ACollisionPoint) <= glm::length(AB) && glm::dot(ACollisionPoint, AB) > 0;
            bool Collision2 = glm::length(CCollisionPoint) <= glm::length(CD) && glm::dot(CCollisionPoint, CD) > 0;
            if(Collision1 && Collision2){
                if(FinalPoint == std::nullopt){
                    FinalPoint = CollisionPoint;
                    obj = object.GetRef();
                }
                else{
                    if(glm::length(A - FinalPoint.value()) > glm::length(A - CollisionPoint)){
                        FinalPoint = CollisionPoint;
                        obj = object.GetRef();
                    }
                }
            }
        }
    }
    if(FinalPoint == std::nullopt){
        lua_newtable(L);
        int MainTable = lua_gettop(L);

        lua_pushboolean(L, false);
        lua_setfield(L, MainTable, "Hit");
    }else{
        lua_newtable(L);
        int MainTable = lua_gettop(L);
        lua_pushboolean(L, true);
        lua_setfield(L, MainTable, "Hit");

        ObjectRef* ud = (ObjectRef*)lua_newuserdata(L, sizeof(ObjectRef));
        *ud = obj;
        luaL_getmetatable(L, "ObjectMetaTable");
        lua_istable(L, -1);
        lua_setmetatable(L, -2);
        lua_setfield(L, MainTable, "Object");

        lua_newtable(L);
        int ChildTable = lua_gettop(L);
        lua_pushnumber(L,FinalPoint.value().x);
        lua_setfield(L, ChildTable, "x");
        lua_pushnumber(L, FinalPoint.value().y);
        lua_setfield(L, ChildTable, "y");
        lua_setfield(L, MainTable, "Point");  
    }
    
    return 1;
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
    rb->Velocity = velocity;
    return 0;
}

int SapphirePhysics::RigidBody::GetVelocity(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    lua_newtable(L);
    lua_pushnumber(L, rb->Velocity.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, rb->Velocity.y);
    lua_setfield(L, -2, "y");
    return 1;
}
