#include "RigidBody.h"
#include "CollisionDetection.h"
#include "Engine/Engine.h"


PhysicsEngine::Body::Body(std::unordered_map<std::string, SapphireEngine::Variable *>& Variables) : Trigger("Trigger", Variables), 
Mass("Mass", Variables), Static("Static", Variables), Restitution("Restitution", Variables),StaticFriction("Static Friction", Variables),DynamicFriction("Dynamic Friction", Variables) 
{
    Variables["Trigger"]->AnyValue() = false;
    Variables["Static"]->AnyValue() = false;
    Variables["Restitution"]->AnyValue() = 0.6f;
    Variables["Static Friction"]->AnyValue() = 0.6f;
    Variables["Dynamic Friction"]->AnyValue() = 0.4f;
    Restitution.Min = 0.0f;
    Restitution.Max = 1.0f;
    DynamicFriction.Min = 0.0f;
    DynamicFriction.Max = 1.0f;
    StaticFriction.Min = 0.0f;
    StaticFriction.Max = 1.0f;
    Restitution.SliderSpeed = 0.01f;
    Variables["Mass"]->AnyValue() = 1.0f;
    Mass.Min = 0.001f;


}

void PhysicsEngine::Body::Update(const float &DeltaTime)
{
    if(Static.value<bool>()) return;
    Force = glm::vec3(0, PhysicsEngine::CollisionDetection::g.value<float>(), 0) * Mass.value<float>();
    Accelaration = (Force/Mass.value<float>());
    if(std::isnan(Accelaration.x) || std::isnan(Accelaration.y) || std::isnan(Accelaration.z))
        Accelaration = glm::vec3(0);
    Velocity += Accelaration * DeltaTime;

    *Position += Velocity * DeltaTime;
    *Rotation += AngularVelocity * DeltaTime;
}

bool PhysicsEngine::Body::CollisionDetection(Object* current)
{
    glm::vec2 Normal;
    float Depth;
    CollisionData CD;
    if(ShapeType == Shapes::RectangleT){
        for (auto&& object: Engine::Get().GetActiveScene()->Objects) {
            if(object->Name == "MainCamera" || object.get() == current) continue;
            if(object->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
                if(PhysicsEngine::CollisionDetection::RectanglexRectangle(object, current,CD)){
                    OnCollisionRotation(current, object.get(), std::move(CD));
                    break;
                }
            }
            else{
                if(PhysicsEngine::CollisionDetection::CirclexRectangle(object.get(), current,CD)){
                    OnCollisionRotation(object.get(), current, std::move(CD));
                    break;
                }
            }
        }
    }
    else{
        for (auto&& object: Engine::Get().GetActiveScene()->Objects) {
            if(object->Name == "MainCamera" || object.get() == current) continue;
            if(object->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
                if(PhysicsEngine::CollisionDetection::CirclexRectangle(current, object.get(),CD)){
                    OnCollisionRotation(current, object.get(), std::move(CD));
                    break;
                }
            }else{
                if(PhysicsEngine::CollisionDetection::CirclexCircle(object, current, CD)){
                    OnCollisionRotation(current, object.get(), std::move(CD));
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
void PhysicsEngine::Body::OnCollisionRotation(Object *current, Object *obj, CollisionData &&CD)
{
    Body* bodyA = &current->GetComponent<RigidBody>()->rb;
    Body* bodyB = &obj->GetComponent<RigidBody>()->rb;

    glm::vec3& bodyAPos = current->GetTransform()->Position.value<glm::vec3>();
    glm::vec3& bodyASize = current->GetTransform()->Size.value<glm::vec3>();
    glm::vec3& bodyBPos = obj->GetTransform()->Position.value<glm::vec3>();
    glm::vec3& bodyBSize = obj->GetTransform()->Size.value<glm::vec3>();
    float bodyAInvMass = 1.0f / bodyA->Mass.value<float>();
    float bodyBInvMass = 1.0f / bodyB->Mass.value<float>();
    float bodyAInvInertia = 1.0f / ((1.0f / 12.0f) * bodyA->Mass.value<float>() * (bodyASize.x * bodyASize.x + bodyASize.y * bodyASize.y));
    float bodyBInvInertia = 1.0f / ((1.0f / 12.0f) * bodyB->Mass.value<float>() * (bodyBSize.x * bodyBSize.x + bodyBSize.y * bodyBSize.y));
    if(bodyA->Static.value<bool>()){
        bodyAInvMass = 0.0f;
        bodyAInvInertia = 0.0f;
    }
    if(bodyB->Static.value<bool>()){
        bodyBInvMass = 0.0f;
        bodyBInvInertia = 0.0f;
    }
    //Not realistic, but good enough for a video game.
    float StaticFriction = (bodyA->StaticFriction.value<float>() + bodyB->StaticFriction.value<float>()) * 0.5f;
    float DynamicFriction = (bodyA->DynamicFriction.value<float>() + bodyB->DynamicFriction.value<float>()) * 0.5f;
    glm::vec2 normal = CD.Normal;
    glm::vec2 contact1 = CD.ContactPoint1;
    glm::vec2 contact2 = CD.ContactPoint2;
    int contactCount = CD.ContactPointCount;

    float e = std::min(bodyA->Restitution.value<float>(), bodyB->Restitution.value<float>());
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
    if(bodyB->Static.value<bool>())
        current->GetTransform()->Position.value<glm::vec3>() += glm::vec3(-CD.Normal * CD.Depth,0);
    else if(bodyA->Static.value<bool>())
        obj->GetTransform()->Position.value<glm::vec3>() += glm::vec3(CD.Normal * CD.Depth,0);
    else{
        current->GetTransform()->Position.value<glm::vec3>() += glm::vec3(-CD.Normal * CD.Depth / 2.0f,0);
        obj->GetTransform()->Position.value<glm::vec3>() += glm::vec3(CD.Normal * CD.Depth / 2.0f,0);
    } 
}

void PhysicsEngine::Body::OnCollision(Object* current, Object* obj, CollisionData&& CD)
{
    obj->OnCollision(current);
    current->OnCollision(obj);


    PhysicsEngine::Body* CurrentRb = &(current->GetComponent<RigidBody>()->rb);
    PhysicsEngine::Body* ObjRb = &(obj->GetComponent<RigidBody>()->rb);
    if(CurrentRb->Static.value<bool>() && ObjRb->Static.value<bool>())
        return;
    
    glm::vec2 RelativeVelocity = ObjRb->Velocity - CurrentRb->Velocity;
    if(glm::dot(RelativeVelocity, CD.Normal) > 0.0f)
        return;

    float CurrentRbInvMass = CurrentRb->Static.value<bool>() ? 0.0f : 1.0f / CurrentRb->Mass.value<float>();
    float ObjRbInvMass = ObjRb->Static.value<bool>() ? 0.0f : 1.0f / ObjRb->Mass.value<float>();

    float e = std::min(CurrentRb->Restitution.value<float>(),ObjRb->Restitution.value<float>());
    float j = -(1.0f + e) * glm::dot(RelativeVelocity, CD.Normal);
    j /= CurrentRbInvMass + ObjRbInvMass;
    glm::vec2 Impulse = j * CD.Normal;

    CurrentRb->Velocity -= glm::vec3(Impulse * CurrentRbInvMass,0);
    ObjRb->Velocity += glm::vec3(Impulse * ObjRbInvMass,0);

    if(CurrentRb-Static.value<bool>())
        current->GetTransform()->Position.value<glm::vec3>() += glm::vec3(-CD.Normal * CD.Depth,0);
    else if(ObjRb-Static.value<bool>())
        obj->GetTransform()->Position.value<glm::vec3>() += glm::vec3(CD.Normal * CD.Depth,0);
    else{
        current->GetTransform()->Position.value<glm::vec3>() += glm::vec3(-CD.Normal * CD.Depth / 2.0f,0);
        obj->GetTransform()->Position.value<glm::vec3>() += glm::vec3(CD.Normal * CD.Depth / 2.0f,0);
    } 
}
