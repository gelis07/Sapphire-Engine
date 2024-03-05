#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"
#include <thread>
#include <execution>
#include <glm/gtx/norm.hpp>
#include "Editor/DebugDraw.h"
#define ITERATIONS 1
#define VECTOR_THRESHOLD 1e-12

b2Vec2 gravity(0.0f, -9.81f);
b2World SapphirePhysics::RigidBody::world(gravity);

void SapphirePhysics::RigidBody::Run()
{
    for (size_t i = 0; i < Rigibodies.size(); i++)
    {
        b2Body* body = Rigibodies[i]->body;
        Transform* transform = Rigibodies[i]->transform;
        b2Fixture* fixture = body->GetFixtureList();
        while (fixture) {
            b2Fixture* nextFixture = fixture->GetNext();
            body->DestroyFixture(fixture);
            fixture = nextFixture;
        }
        // Define a box shape for the dynamic body
        b2PolygonShape Box;
        const glm::vec3 size = glm::vec3(abs(transform->GetSize().x) / 2.0f, abs(transform->GetSize().y)  / 2.0f, 0.0f);
        Box.SetAsBox(size.x, size.y); // Box dimensions

        float area = size.x * size.y;
        float den = Rigibodies[i]->Mass.Get() / area;
        // Define fixture
        b2FixtureDef dynamicFixtureDef;
        dynamicFixtureDef.shape = &Box;
        dynamicFixtureDef.restitution = 0.0f;
        dynamicFixtureDef.density = den / 4.0f;
        dynamicFixtureDef.friction = Rigibodies[i]->StaticFriction.Get();

        // Attach fixture to dynamic body
        body->CreateFixture(&dynamicFixtureDef);
        const glm::vec3 Pos = Rigibodies[i]->transform->GetPosition();
        body->SetTransform(b2Vec2(Pos.x, Pos.y), Rigibodies[i]->transform->GetRotation().z);
    }
    world.Step(FixedTimeStep, velocityIterations, positionIterations);
    for (size_t i = 0; i < Rigibodies.size(); i++)
    {
        b2Body* body = Rigibodies[i]->body;
        Rigibodies[i]->transform->SetPosition(glm::vec3(body->GetPosition().x,body->GetPosition().y,0.0f));
        Rigibodies[i]->transform->SetRotation(body->GetAngle());
    }
}

SapphirePhysics::RigidBody::RigidBody(int st, ObjectRef obj) : Trigger("Trigger", Variables),
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("StaticFriction", Variables), DynamicFriction("DynamicFriction", Variables), Rotate("Rotate", Variables), Kinematic("Kinematic", Variables),
Component("Rigidbody", obj)
{
    ShapeType = st;
    Trigger.Get() = false;
    Static.Get() = false;
    Rotate.Get() = true;
    Kinematic.Get() = false;
    Restitution.Get() = 0.0f;
    StaticFriction.Get() = 0.0f;
    DynamicFriction.Get() = 0.0f;
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
    Functions["SetMass"] = SetMass;
    Functions["SetFriction"] = SetFriction;
    Functions["SetVelocity"] = SetVelocity;
    Functions["GetVelocity"] = GetVelocity;
    Functions["Raycast"] = RayCast;
    {
        std::function<void()> OnChange = [this]() {
            if(Static.Get() == true){
                body->SetType(b2_staticBody);
            }else{
                body->SetType(b2_dynamicBody);
            }
        };
        Static.SetOnChangeFunc(OnChange);
    }
    {
        std::function<void()> OnChange = [this]() {
            if(Kinematic.Get() == true){
                body->SetType(b2_kinematicBody);
            }else{
                body->SetType(b2_dynamicBody);
            }
        };
        Kinematic.SetOnChangeFunc(OnChange);
    }
    {
        std::function<void()> OnChange = [this]() {
            body->SetFixedRotation(!(Rotate.Get()));
        };
        Rotate.SetOnChangeFunc(OnChange);
    }
}

SapphirePhysics::RigidBody::RigidBody(const RigidBody &rb): Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),Kinematic("Kinematic", Variables),Rotate("Rotate", Variables),
Component("Rigidbody", rb.Parent)
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
    Functions["SetMass"] = SetMass;
    Functions["SetFriction"] = SetFriction;
    Functions["SetVelocity"] = SetVelocity;
    Functions["GetVelocity"] = GetVelocity;
    Functions["Raycast"] = RayCast;
}

void SapphirePhysics::RigidBody::Init()
{
    if(body != nullptr){
        SapphirePhysics::RigidBody::world.DestroyBody(body);
    }
    b2BodyDef BodyDef;
    BodyDef.type = b2_dynamicBody;
    BodyDef.position.Set(transform->GetPosition().x, transform->GetPosition().y); // Set the initial position
    body = RigidBody::world.CreateBody(&BodyDef);


    // Define a box shape for the dynamic body
    b2PolygonShape Box;
    const glm::vec3 size = glm::vec3(abs(transform->GetSize().x) / 2.0f, abs(transform->GetSize().y)  / 2.0f, 0.0f);
    Box.SetAsBox(size.x, size.y); // Box dimensions

    float area = size.x * size.y;
    float den = Mass.Get() / area;
    // Define fixture
    b2FixtureDef dynamicFixtureDef;
    dynamicFixtureDef.shape = &Box;
    dynamicFixtureDef.restitution = 0.0f;
    dynamicFixtureDef.density = den / 4.0f;
    dynamicFixtureDef.friction = StaticFriction.Get();

    // Attach fixture to dynamic body
    body->CreateFixture(&dynamicFixtureDef);
    body->SetFixedRotation(!(Rotate.Get()));
    // Reset mass data to recalculate mass properties
    // body->ResetMassData();
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

void SapphirePhysics::RigidBody::Update(float DeltaTime)
{
    if(Static.Get()) return;
    Forces.push_back(glm::vec3(0, SapphirePhysics::CollisionDetection::g.Get(), 0) * Mass.Get());
    Accelaration = (SapphireEngine::VectorSum(Forces)/Mass.Get());
    AngularAccelaration = SapphireEngine::VectorSum(Torques) / (((1.0f / 12.0f) * Mass.Get() * (transform->GetSize().x * transform->GetSize().x + transform->GetSize().y * transform->GetSize().y)));
    Velocity += Accelaration * DeltaTime;
    AngularVelocity.z += AngularAccelaration.z * DeltaTime;
    transform->Move(Velocity * DeltaTime);
    if(Rotate.Get())
        transform->Rotate(AngularVelocity.z * DeltaTime);
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
    PROFILE_FUNC();
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
    PROFILE_FUNC();
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

// Thanks to the book "Physics for Game Developers" from David M. Bourg and Bryan Bywalec.
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
    // float bodyAInvInertia = 1.0f / (bodyASize.x * pow(bodyASize.y, 3));
    // float bodyBInvInertia = 1.0f / (bodyBSize.x * pow(bodyBSize.y, 3));
    if(bodyA->Static.Get()){
        bodyAInvMass = 0;
        bodyAInvInertia = 0;
    }
    if(bodyB->Static.Get()){
        bodyBInvMass = 0;
        bodyBInvInertia = 0;
    }

    std::array<glm::vec2, 2> contactList {glm::vec2(0),glm::vec2(0)};
    std::array<glm::vec3, 2> raList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> rbList = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> VpA = {glm::vec3(0),glm::vec3(0)};
    std::array<glm::vec3, 2> VpB = {glm::vec3(0),glm::vec3(0)};
    std::array<float, 2> JList;
    contactList[0] = CD.ContactPoint1;
    contactList[1] = CD.ContactPoint2;
    //coefficient of restitution.
    float e = std::max(bodyA->Restitution.Get(), bodyB->Restitution.Get());
    //coefficient of friction.
    float cf = (bodyA->StaticFriction.Get() + bodyB->StaticFriction.Get()) * 0.5f;

    glm::vec3 n = glm::vec3(CD.Normal,0);
    n = glm::normalize(n);
    for (size_t i = 0; i < CD.ContactPointCount; i++)
    {
        glm::vec2 Point = contactList[i];
        glm::vec3 r1 = glm::vec3(Point - glm::vec2(bodyAPos), 0);
        glm::vec3 r2 = glm::vec3(Point - glm::vec2(bodyBPos), 0);
        // SapphireEngine::AddLine(glm::vec2(bodyAPos), glm::vec2(bodyAPos) + glm::vec2(r1), glm::vec4(1,0,0,1), 5.0f);
        // SapphireEngine::AddLine(glm::vec2(bodyBPos), glm::vec2(bodyBPos) + glm::vec2(r2), glm::vec4(0,1,0,1), 5.0f);
        glm::vec3 Vp1 = bodyA->Velocity + glm::cross(bodyA->AngularVelocity, r1);
        glm::vec3 Vp2 = bodyB->Velocity + glm::cross(bodyB->AngularVelocity, r2);
        glm::vec3 RelativeVelocity = Vp1 - Vp2;
        // float J = -glm::dot(RelativeVelocity, n) * (e + 1) 
        // / (bodyAInvMass + bodyBInvMass 
        // + glm::dot(n, glm::cross(glm::cross(r1, n) * bodyAInvMass, r1))
        // + glm::dot(n, glm::cross(glm::cross(r2, n) * bodyBInvInertia, r2)));
        float J = -glm::length(RelativeVelocity)*(e+1)/(bodyAInvMass + bodyBInvMass);

        JList[i] = J / 2.0f;
        raList[i] = r1;
        rbList[i] = r2;
        VpA[i] = Vp1;
        VpB[i] = Vp2;
    }
    if(bodyB->Static.Get()){
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth,0));
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            glm::vec3 RelVel = VpB[i] - VpA[i];
            glm::vec3 t = glm::cross(glm::cross(n, RelVel), n);
            if(t != glm::vec3(0))
                t = glm::normalize(t);
            // SapphireEngine::AddLine(contactList[i], contactList[i] + glm::vec2(t) * 3.0f, glm::vec4(1, 0,0,1), 5.0f);


            glm::vec3 frictionImpulse = cf * JList[i]*t / FixedTimeStep;
            glm::vec3 maxTangentialFrictionForce = cf * (JList[i] * n) / FixedTimeStep;
            glm::vec3 Impulse = JList[i]*n/FixedTimeStep;

            if (glm::length(frictionImpulse) > glm::length(maxTangentialFrictionForce)){
                frictionImpulse = maxTangentialFrictionForce;
            }
            glm::vec3 NewVel = (JList[i] * n + cf * JList[i] * t) * bodyAInvMass;
            glm::vec3 NewAnglVel = glm::cross(raList[i], JList[i] * n) * bodyAInvInertia;
            bodyA->Velocity += (JList[i] * n + cf * JList[i] * t) * bodyAInvMass;
            bodyA->AngularVelocity += glm::cross(raList[i], JList[i] * n) * bodyAInvInertia;
            // std::cout << "Velocity: x: " << NewVel.x << ", y: " << NewVel.y << '\n';
            // std::cout << "Angular Velocity: z: " << NewAnglVel.z << '\n';
        }
        
    }
    else if(bodyA->Static.Get()){
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth,0));
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            glm::vec3 RelVel = VpA[i] - VpB[i];
            glm::vec3 t = glm::cross(glm::cross(n, RelVel), n);
            if(t != glm::vec3(0))
                t = glm::normalize(t);
            // SapphireEngine::AddLine(contactList[i], contactList[i] + glm::vec2(n) * 3.0f, glm::vec4(1, 0,0,1), 5.0f);
            glm::vec3 frictionImpulse = cf * JList[i]*t / FixedTimeStep;
            glm::vec3 maxTangentialFrictionForce = cf * (JList[i] * n) / FixedTimeStep;
            glm::vec3 Impulse = JList[i]*n/FixedTimeStep;

            if (glm::length(frictionImpulse) > glm::length(maxTangentialFrictionForce)){
                frictionImpulse = maxTangentialFrictionForce;
            }
            glm::vec3 NewVel = (-JList[i] * n + cf * JList[i] * t) * bodyBInvMass;
            glm::vec3 NewAnglVel = glm::cross(rbList[i], -JList[i] * n) * bodyBInvInertia;

            bodyB->Velocity += (-JList[i] * n + cf * -JList[i] * t) * bodyBInvMass;
            bodyB->AngularVelocity += glm::cross(rbList[i], -JList[i] * n) * bodyBInvInertia;
            // std::cout << "Velocity: x: " << NewVel.x << ", y: " << NewVel.y << '\n';
            // std::cout << "Angular Velocity: z: " << NewAnglVel.z << '\n';
        }
    }
    else{
        bodyA->transform->Move(glm::vec3(-CD.Normal * CD.Depth / 2.0f,0));
        bodyB->transform->Move(glm::vec3(CD.Normal * CD.Depth / 2.0f,0));

        // SapphireEngine::AddLine(glm::vec2(bodyAPos), glm::vec2(bodyAPos) + glm::vec2(bodyA->Forces.back()), glm::vec4(1,0,0,1), 5.0f);
        // SapphireEngine::AddLine(glm::vec2(bodyBPos), glm::vec2(bodyBPos) + glm::vec2(bodyB->Forces.back()), glm::vec4(0,0,1,1), 5.0f);
        for (size_t i = 0; i < CD.ContactPointCount; i++)
        {
            glm::vec3 RelVel = VpA[i] - VpB[i];
            glm::vec3 t = glm::cross(glm::cross(n, RelVel), n);
            if(t != glm::vec3(0))
                t = glm::normalize(t);
            // SapphireEngine::AddLine(contactList[i], contactList[i] + glm::vec2(n) * 3.0f, glm::vec4(1, 0,0,1), 5.0f);
            glm::vec3 frictionImpulse = cf * JList[i]*t / FixedTimeStep;
            glm::vec3 maxTangentialFrictionForce = cf * (JList[i] * n) / FixedTimeStep;
            glm::vec3 Impulse = JList[i]*n/FixedTimeStep;

            if (glm::length(frictionImpulse) > glm::length(maxTangentialFrictionForce)){
                frictionImpulse = maxTangentialFrictionForce;
            }
            bodyA->Velocity += (JList[i] * n + cf * JList[i] * t) * bodyAInvMass;
            bodyA->AngularVelocity += glm::cross(raList[i], JList[i] * n + cf * JList[i] * t) * bodyAInvInertia;
            bodyB->Velocity += (-JList[i] * n + cf * JList[i] * t) * bodyBInvMass;
            bodyB->AngularVelocity += glm::cross(rbList[i], -JList[i] * n + cf * JList[i] * t) * bodyBInvInertia;
        }
        
    } 
}

void SapphirePhysics::RigidBody::Simulate(int Index) {
    PROFILE_FUNC();
    for (auto &object : Engine::GetActiveScene().Objects)
    {
        object.CalledOnCollision = false;
    }
    for (size_t i = 1; i <= ITERATIONS; i++)
    {
        ContactPairs.clear();
        Update(FixedTimeStep/ITERATIONS);
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
    rb->body->ApplyLinearImpulse(b2Vec2(x,y),b2Vec2(rb->transform->GetPosition().x, rb->transform->GetPosition().y), true);
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
    lua_getfield(L, -1, "x");
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "y");
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    rb->body->SetLinearVelocity(b2Vec2(x,y));
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

int SapphirePhysics::RigidBody::SetFriction(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float friction = (float)luaL_checknumber(L, 2);
    rb->StaticFriction.Get() = friction;
    Transform* transform = rb->transform;
    b2Body* body = rb->body;
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture) {
        b2Fixture* nextFixture = fixture->GetNext();
        body->DestroyFixture(fixture);
        fixture = nextFixture;
    }
    // Define a box shape for the dynamic body
    b2PolygonShape Box;
    const glm::vec3 size = glm::vec3(abs(transform->GetSize().x) / 2.0f, abs(transform->GetSize().y)  / 2.0f, 0.0f);
    Box.SetAsBox(size.x, size.y); // Box dimensions

    float area = size.x * size.y;
    float den = rb->Mass.Get() / area;
    // Define fixture
    b2FixtureDef dynamicFixtureDef;
    dynamicFixtureDef.shape = &Box;
    dynamicFixtureDef.density = den / 4.0f;
    dynamicFixtureDef.restitution = 0.0f;
    dynamicFixtureDef.friction = rb->StaticFriction.Get();

    // Attach fixture to dynamic body
    body->CreateFixture(&dynamicFixtureDef);
    return 0;
}

int SapphirePhysics::RigidBody::SetKinematic(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    rb->Kinematic.Get() = true;
    rb->body->SetType(b2_kinematicBody);
    return 0;
}

int SapphirePhysics::RigidBody::SetMass(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    SapphirePhysics::RigidBody* rb = static_cast<SapphirePhysics::RigidBody*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float mass = (float)luaL_checknumber(L, 2);
    rb->Mass.Get() = mass;
    Transform* transform = rb->transform;
    b2Body* body = rb->body;
    b2Fixture* fixture = body->GetFixtureList();
    while (fixture) {
        b2Fixture* nextFixture = fixture->GetNext();
        body->DestroyFixture(fixture);
        fixture = nextFixture;
    }
    // Define a box shape for the dynamic body
    b2PolygonShape Box;
    const glm::vec3 size = glm::vec3(abs(transform->GetSize().x) / 2.0f, abs(transform->GetSize().y)  / 2.0f, 0.0f);
    Box.SetAsBox(size.x, size.y); // Box dimensions

    float area = size.x * size.y;
    float den = rb->Mass.Get() / area;
    // Define fixture
    b2FixtureDef dynamicFixtureDef;
    dynamicFixtureDef.shape = &Box;
    dynamicFixtureDef.restitution = 0.0f;
    dynamicFixtureDef.density = den / 4.0f;
    dynamicFixtureDef.friction = rb->StaticFriction.Get();

    // Attach fixture to dynamic body
    body->CreateFixture(&dynamicFixtureDef);
    return 0;
}
