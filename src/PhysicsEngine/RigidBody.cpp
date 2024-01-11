#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"
#include <thread>
#include <execution>
#include <glm/gtx/norm.hpp>
#include "Editor/DebugDraw.h"
#define ITERATIONS 1
#define VECTOR_THRESHOLD 1e-12

SapphirePhysics::RigidBody::RigidBody(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp) : Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),
    Component(std::move(File), std::move(ArgName), ArgId,LuaComp)
{
    Trigger.Get() = false;
    Static.Get() = false;
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
    Functions["GetVelocity"] = GetVelocity;
    Functions["Raycast"] = RayCast;
}

void SapphirePhysics::RigidBody::Update(const float &DeltaTime)
{
    if(Static.Get()) return;
    Forces.push_back(glm::vec3(0, SapphirePhysics::CollisionDetection::g.Get(), 0) * Mass.Get());
    Accelaration = (SapphireEngine::VectorSum(Forces)/Mass.Get());
    AngularAccelaration = SapphireEngine::VectorSum(Torques) / (((1.0f / 12.0f) * Mass.Get() * (transform->GetSize().x * transform->GetSize().x + transform->GetSize().y * transform->GetSize().y)));
    Velocity += StartingVelocity + Accelaration * FixedTimeStep;
    AngularVelocity.z += AngularAccelaration.z * FixedTimeStep;
    // std::stringstream ss;
    // ss << "Velocity x: " << Velocity.x << ", y: " << Velocity.y << '\n';
    // ss << "Accelaration x: " << Accelaration.x << "y: " << Accelaration.y;
    // SapphireEngine::Log(ss.str());
    // if(glm::length2(Velocity) <= 0.1f) Velocity = glm::vec3(0);
    transform->Move(Velocity * FixedTimeStep);
    transform->Rotate(AngularVelocity.z * FixedTimeStep);
    StartingVelocity = glm::vec3(0);
    Forces.clear();
    Torques.clear();
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
    int index = Engine::GetActiveScene().ObjectRefrences[current->GetRefID()];
    if(current->GetRb()->ShapeType == SapphireRenderer::RectangleT){
        std::for_each(std::execution::par,(Engine::GetActiveScene().Objects.begin()+index), Engine::GetActiveScene().Objects.end(), [current](auto&& object) {
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
    // std::for_each(std::execution::par,ContactPairs.begin(), ContactPairs.end(), [](auto&& pair) {
    //     CollisionData CD;
    //     std::shared_ptr<Object> sharedObject(pair.second,StackObjectDeleter{});
    //     if(SapphirePhysics::CollisionDetection::RectanglexRectangle(sharedObject, pair.first,CD)){
    //         pair.first->OnCollision(pair.second);
    //         pair.second->OnCollision(pair.first);
    //         OnCollisionRotation(pair.first, pair.second, std::move(CD));
    //     }
    // });

    for (auto &&pair : ContactPairs)
    {
        std::stringstream ss;
        CollisionData CD;
        std::shared_ptr<Object> sharedObject(pair.second,StackObjectDeleter{});
        if(SapphirePhysics::CollisionDetection::RectanglexRectangle(sharedObject, pair.first,CD)){
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
    // glm::vec3 RelativeVelocity = bodyB->Velocity - bodyA->Velocity;
    // float J = glm::length(RelativeVelocity) * ((0.2f) + 1) / (bodyAInvMass + bodyBInvMass);
    // glm::vec2 Point = CD.ContactPoint1 + (CD.ContactPoint2 - CD.ContactPoint1) / 2.0f;
    // if(glm::vec2(Point - CD.ContactPoint2).x < 0 && glm::vec2(Point - CD.ContactPoint1).x < 0 && glm::dot(glm::normalize(glm::vec2(Point - CD.ContactPoint2)), glm::normalize(glm::vec2(Point - CD.ContactPoint1))) == 1){
    //     Point = CD.ContactPoint2;
    // }
    // glm::vec2 Point = glm::vec2(CD.ContactPoint1.x + (CD.ContactPoint2.x - CD.ContactPoint1.x) / 2.0f, bodyAPos.y);
    // if(CD.ContactPointCount == 1){
    //     Point = glm::vec2(CD.ContactPoint1.x, bodyAPos.y);
    // }
    // glm::vec2 PointA = (CD.ContactPoint1 + CD.ContactPoint2) / 2.0f - glm::vec2(bodyAPos);
    // glm::vec2 PointB = (CD.ContactPoint1 + CD.ContactPoint2) / 2.0f - glm::vec2(bodyBPos);
    // SapphireEngine::Log(std::to_string(r1.x) + ", " + std::to_string(r1.y));
    // SapphireEngine::AddLine(glm::vec2(0,0), Point, glm::vec4(0,0,1,1), 5.0f);
    // SapphireEngine::AddLine(glm::vec2(0,0), CD.ContactPoint2, glm::vec4(0,1,0,1), 5.0f);
    // SapphireEngine::AddLine(glm::vec2(0,0), CD.ContactPoint1, glm::vec4(0,1,1,1), 5.0f);
    // SapphireEngine::AddLine(glm::vec2(0,0), glm::vec2(bodyAPos), glm::vec4(1,0,0,1), 5.0f);

    std::array<glm::vec2, 2> contactList {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec3, 2> FrictionImpulseList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> raList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> rbList = {glm::vec3(0),glm::vec3(0)};
    std::array<float, 2> JList;
    contactList[0] = CD.ContactPoint1;
    contactList[1] = CD.ContactPoint2;
    float e = std::min(bodyA->Restitution.Get(), bodyB->Restitution.Get());
    float cf = (bodyA->StaticFriction.Get() + bodyB->StaticFriction.Get()) * 0.5f;;
    // glm::vec3 n = glm::vec3(0,1,0);
    // // n = glm::vec3(-n.y, n.x, n.z);

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

    }
    glm::vec3 t = glm::normalize(bodyA->Velocity - bodyB->Velocity);

    if(bodyB->Static.Get()){
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth,0));
        bodyA->Forces.push_back(JList[0]*n/FixedTimeStep);
        bodyA->Forces.push_back(cf * JList[0]*t /FixedTimeStep);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            bodyA->Torques.push_back(glm::cross(raList[i], (JList[i] * n) + (cf*JList[i])*t));
        }
        
    }
    else if(bodyA->Static.Get()){
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth,0));
        bodyB->Forces.push_back(-JList[0]*n/FixedTimeStep);
        bodyB->Forces.push_back(cf * -JList[0]*t /FixedTimeStep);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            bodyB->Torques.push_back(glm::cross(rbList[i], -JList[i] * n + (cf*JList[i]*t)));
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
            // bodyA->Torques.push_back(glm::cross(raList[i], JList[i] * n));
            // bodyB->Torques.push_back(glm::cross(rbList[i], -JList[i] * n));
            bodyA->Torques.push_back(glm::cross(raList[i], (JList[i] * n) + (cf*JList[i])*t));
            bodyB->Torques.push_back(glm::cross(rbList[i], -JList[i] * n + (cf*JList[i]*t)));
        }
        
    } 
}
void SapphirePhysics::RigidBody::Simulate(Object* current, const float& DeltaTime) {
    for (auto &object : Engine::GetActiveScene().Objects)
    {
        object.CalledOnCollision = false;
    }
    for (size_t i = 1; i <= ITERATIONS; i++)
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
        if(object.GetRb().get() == rb) continue;
        if(tag != "" && object.Tag != tag) continue;
        for (size_t i = 0; i < object.GetTransform()->GetPoints().size(); i++)
        {
            glm::vec2 C = object.GetTransform()->GetPoints()[i];
            glm::vec2 D = object.GetTransform()->GetPoints()[(i + 1) % object.GetTransform()->GetPoints().size()];
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
