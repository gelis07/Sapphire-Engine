#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"


SapphirePhysics::RigidBody::RigidBody(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp) : Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),
StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables),
    Component(std::move(File), std::move(ArgName), ArgId,LuaComp)
{
    Trigger.Get() = false;
    Static.Get() = false;
    Restitution.Get() = 0.6f;
    StaticFriction.Get() = 0.6f;
    DynamicFriction.Get() = 0.4f;
    Restitution.Min = 0.0f;
    Restitution.Max = 1.0f;
    DynamicFriction.Min = 0.0f;
    DynamicFriction.Max = 1.0f;
    StaticFriction.Min = 0.0f;
    StaticFriction.Max = 1.0f;
    Restitution.SliderSpeed = 0.01f;
    Mass.Get() = 1.0f;
    Mass.Min = 0.001f;
    Functions["Impulse"] = Impulse;
}

void SapphirePhysics::RigidBody::Update(const float &DeltaTime)
{
    if(Static.Get()) return;
    Force = glm::vec3(0, SapphirePhysics::CollisionDetection::g.Get(), 0) * Mass.Get();
    Accelaration = (Force/Mass.Get());
    if(std::isnan(Accelaration.x) || std::isnan(Accelaration.y) || std::isnan(Accelaration.z))
        Accelaration = glm::vec3(0);
    Velocity += Accelaration * DeltaTime;

    transform->Move(Velocity * DeltaTime);
    transform->Rotate(AngularVelocity.z * DeltaTime);
}
struct StackObjectDeleter {
    void operator()(Object* /* object */) const {
        // Custom deleter, no need to delete stack-allocated objects
    }
};
bool SapphirePhysics::RigidBody::CollisionDetection(Object* current)
{
    glm::vec2 Normal;
    float Depth;
    CollisionData CD;
    if(ShapeType == SapphireRenderer::RectangleT){
        for (auto&& object: Engine::GetActiveScene().Objects) {
            if(object.Name == "MainCamera" || &object == current) continue;
            //! FOR SOME REASON IT WORKS WITH SHARED POITNERS ONLY I HAVE NO IDEA WHY.
            std::shared_ptr<Object> sharedObject(&object,StackObjectDeleter{});
            if(object.GetComponent<Renderer>()->shape->ShapeType == SapphireRenderer::RectangleT){
                if(SapphirePhysics::CollisionDetection::RectanglexRectangle(sharedObject, current,CD)){
                    OnCollisionRotation(current, &object, std::move(CD));
                    break;
                }
            }
            else{
                if(SapphirePhysics::CollisionDetection::CirclexRectangle(&object, current,CD)){
                    OnCollisionRotation(&object, current, std::move(CD));
                    break;
                }
            }
        }
    }
    else{
        for (auto&& object: Engine::GetActiveScene().Objects) {
            if(object.Name == "MainCamera" || &object == current) continue;
            std::shared_ptr<Object> sharedObject(&object);
            if(object.GetComponent<Renderer>()->shape->ShapeType == SapphireRenderer::RectangleT){
                if(SapphirePhysics::CollisionDetection::CirclexRectangle(current, &object,CD)){
                    OnCollisionRotation(current, &object, std::move(CD));
                    break;
                }
            }else{
                if(SapphirePhysics::CollisionDetection::CirclexCircle(sharedObject, current, CD)){
                    OnCollisionRotation(current, &object, std::move(CD));
                    break;
                }
            }
        }
    }
    return false;
}


float CrossProduct(glm::vec2 v1, glm::vec2 v2){
    return v1.x * v2.y - v1.y * v2.x;
}
void SapphirePhysics::RigidBody::OnCollisionRotation(Object *current, Object *obj, CollisionData &&CD)
{
    RigidBody* bodyA = current->GetComponent<RigidBody>().get();
    RigidBody* bodyB = obj->GetComponent<RigidBody>().get();

    const glm::vec3& bodyAPos = current->GetTransform()->GetPosition();
    const glm::vec3& bodyASize = current->GetTransform()->GetSize();
    const glm::vec3& bodyBPos = obj->GetTransform()->GetPosition();
    const glm::vec3& bodyBSize = obj->GetTransform()->GetSize();
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
        j /= denom;
        j /= (float)contactCount;
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
        bodyB->Velocity += glm::vec3(impulse * bodyBInvMass,0);
        bodyB->AngularVelocity.z += CrossProduct(rb, impulse) * bodyBInvInertia;
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
            tangent = glm::normalize(tangent);
        }
        float raPerpDotT = glm::dot(raPerp, tangent);
        float rbPerpDotT = glm::dot(rbPerp, tangent);

        float denom = bodyAInvMass + bodyBInvMass + 
            (raPerpDotT * raPerpDotT) * bodyAInvInertia + 
            (rbPerpDotT * rbPerpDotT) * bodyBInvInertia;

        float jt = -glm::dot(relativeVelocity, tangent);
        jt /= denom;
        jt /= (float)contactCount;
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
        bodyB->Velocity += glm::vec3(impulse * bodyBInvMass,0);
        bodyB->AngularVelocity.z += CrossProduct(glm::normalize(rb), impulse) * bodyBInvInertia;
    }
    if(bodyB->Static.Get())
        current->GetTransform()->Move(glm::vec3(-CD.Normal * CD.Depth,0));
    else if(bodyA->Static.Get())
        obj->GetTransform()->Move(glm::vec3(CD.Normal * CD.Depth,0));
    else{
        current->GetTransform()->Move(glm::vec3(-CD.Normal * CD.Depth / 2.0f,0));
        obj->GetTransform()->Move(glm::vec3(CD.Normal * CD.Depth / 2.0f,0));
    } 
}

void SapphirePhysics::RigidBody::OnCollision(Object* current, Object* obj, CollisionData&& CD)
{
    obj->OnCollision(current);
    current->OnCollision(obj);


    SapphirePhysics::RigidBody* CurrentRb = current->GetComponent<RigidBody>().get();
    SapphirePhysics::RigidBody* ObjRb = obj->GetComponent<RigidBody>().get();
    if(CurrentRb->Static.Get() && ObjRb->Static.Get())
        return;
    
    glm::vec2 RelativeVelocity = ObjRb->Velocity - CurrentRb->Velocity;
    if(glm::dot(RelativeVelocity, CD.Normal) > 0.0f)
        return;

    float CurrentRbInvMass = CurrentRb->Static.Get() ? 0.0f : 1.0f / CurrentRb->Mass.Get();
    float ObjRbInvMass = ObjRb->Static.Get() ? 0.0f : 1.0f / ObjRb->Mass.Get();

    float e = std::min(CurrentRb->Restitution.Get(),ObjRb->Restitution.Get());
    float j = -(1.0f + e) * glm::dot(RelativeVelocity, CD.Normal);
    j /= CurrentRbInvMass + ObjRbInvMass;
    glm::vec2 Impulse = j * CD.Normal;

    CurrentRb->Velocity -= glm::vec3(Impulse * CurrentRbInvMass,0);
    ObjRb->Velocity += glm::vec3(Impulse * ObjRbInvMass,0);

    if(CurrentRb-Static.Get())
        current->GetTransform()->Move(glm::vec3(-CD.Normal * CD.Depth,0));
    else if(ObjRb-Static.Get())
        obj->GetTransform()->Move(glm::vec3(CD.Normal * CD.Depth,0));
    else{
        current->GetTransform()->Move(glm::vec3(-CD.Normal * CD.Depth / 2.0f,0));
        obj->GetTransform()->Move(glm::vec3(CD.Normal * CD.Depth / 2.0f,0));
    } 
}


void SapphirePhysics::RigidBody::Simulate(Object *current, const float& DeltaTime) {
    Update(DeltaTime);
    CollisionDetection(current);
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
