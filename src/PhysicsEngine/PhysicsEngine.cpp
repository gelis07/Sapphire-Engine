#include "PhysicsEngine.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "UI/Windows.h" 

SapphireEngine::Float PhysicsEngine::g("g", Windows::SettingsVariables);


bool PhysicsEngine::CirclexRectangle(std::shared_ptr<Object> obj, Object* current)
{
    // These points are linked to the geogebra files!
    float Cy = obj->GetTransform()->Position.value<glm::vec3>().y - obj->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the bottom Left point of the rectangle
    float Ay = obj->GetTransform()->Position.value<glm::vec3>().y + obj->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the top Left point of the rectangle
    float Ax = obj->GetTransform()->Position.value<glm::vec3>().x - obj->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top Left point of the rectangle
    float Bx = obj->GetTransform()->Position.value<glm::vec3>().x + obj->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top right point of the rectangle

    // The point which the circle is going to hit (lets call it point T)
    glm::vec2 CollisionPoint(SapphireEngine::ClampFunc(Ax, Bx, current->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(Cy, Ay, current->GetTransform()->Position.value<glm::vec3>()[1]));

    // Checking if the length of the vector with points T and the circle's position is less than the radius
    if (SapphireEngine::LengthVec(CollisionPoint, current->GetTransform()->Position.value<glm::vec3>()) <= current->GetTransform()->Size.value<glm::vec3>().x / 2)
    {
        //A collision is occuring
        obj->OnCollision(current);
        current->OnCollision(obj.get());
        return true;
    //    if(obj->GetTrigger()) return;
    //    float diff = current->GetTransform()->Si.value<glm::vec3>().x / 2 - SapphireEngine::LengthVec(CollisionPoint, current->GetTransform()->Position.value<glm::vec3>());
    //    glm::vec2 NormalizedT((CollisionPoint.x - current->GetTransform()->Position.x) / SapphireEngine::LengthVec(CollisionPoint, current->GetTransform()->Position.value<glm::vec3>()), (CollisionPoint.y - current->GetTransform()->Position.y) / SapphireEngine::LengthVec(CollisionPoint, current->GetTransform()->Position));

    //    glm::vec3 VectorDiff(NormalizedT * diff, 0.0f);
    //    if (isnan(NormalizedT.x))
    //    {
    //        VectorDiff = glm::vec3(0, 0, 0);
    //    }
    //    current->SetPos(current->GetTransform()->Position - VectorDiff);
    }
    return false;

}
bool PhysicsEngine::RectanglexRectangle(std::shared_ptr<Object> obj, Object* current)
{
    float& ObjRot = obj->GetTransform()->Rotation.value<glm::vec3>().z;
    glm::vec3& ObjPos = obj->GetTransform()->Position.value<glm::vec3>();
    glm::vec3& ObjSize = obj->GetTransform()->Size.value<glm::vec3>();
    // These points are linked to the geogebra files!
    //! Should be stored in the renderer in memory
    glm::vec2 F = glm::vec2(ObjPos.x - ObjSize.x / 2, ObjPos.y + ObjSize.y / 2);
    glm::vec2 E = glm::vec2(ObjPos.x + ObjSize.x / 2, ObjPos.y + ObjSize.y / 2);
    glm::vec2 H = glm::vec2(ObjPos.x - ObjSize.x / 2, ObjPos.y - ObjSize.y / 2);
    glm::vec2 G = glm::vec2(ObjPos.x + ObjSize.x / 2, ObjPos.y - ObjSize.y / 2);

    F = glm::vec2((F.x) * cos((ObjRot)) + (F.y) * (-sin((ObjRot))), (F.x) * sin((ObjRot)) + (F.y) * cos((ObjRot))); // The y of the bottom Left point of the "Other" Rectangle
    E = glm::vec2((E.x) * cos((ObjRot)) + (E.y) * (-sin((ObjRot))), (E.x) * sin((ObjRot)) + (E.y) * cos((ObjRot))); // The y of the top Left point of the "Other" Rectangle
    H = glm::vec2((H.x) * cos((ObjRot)) + (H.y) * (-sin((ObjRot))), (H.x) * sin((ObjRot)) + (H.y) * cos((ObjRot))); // The x of the top Left point of the "Other" Rectangle
    G = glm::vec2((G.x) * cos((ObjRot)) + (G.y) * (-sin((ObjRot))), (G.x) * sin((ObjRot)) + (G.y) * cos((ObjRot))); // The x of the top right point of the "Other" Rectangle


    float& CurrentRot = obj->GetTransform()->Rotation.value<glm::vec3>().z;
    glm::vec3& CurrentPos = current->GetTransform()->Position.value<glm::vec3>();
    glm::vec3& CurrentSize = current->GetTransform()->Size.value<glm::vec3>();

    glm::vec2 A = glm::vec2(CurrentPos.x + CurrentSize.x / 2, CurrentPos.y + CurrentSize.y / 2); 
    glm::vec2 B = glm::vec2(CurrentPos.x + CurrentSize.x / 2, CurrentPos.y - CurrentSize.y / 2); 
    glm::vec2 C = glm::vec2(CurrentPos.x - CurrentSize.x / 2, CurrentPos.y + CurrentSize.y / 2); 
    glm::vec2 D = glm::vec2(CurrentPos.x - CurrentSize.x / 2, CurrentPos.y - CurrentSize.y / 2); 

    A = glm::vec2((A.x) * cos((CurrentRot)) + (A.y) * (-sin((CurrentRot))), (A.x) * sin((CurrentRot)) + (A.y) * cos((CurrentRot))); // The y of the bottom Left point of the "Other" Rectangle
    B = glm::vec2((B.x) * cos((CurrentRot)) + (B.y) * (-sin((CurrentRot))), (B.x) * sin((CurrentRot)) + (B.y) * cos((CurrentRot))); // The y of the top Left point of the "Other" Rectangle
    C = glm::vec2((C.x) * cos((CurrentRot)) + (C.y) * (-sin((CurrentRot))), (C.x) * sin((CurrentRot)) + (C.y) * cos((CurrentRot))); // The x of the top Left point of the "Other" Rectangle
    D = glm::vec2((D.x) * cos((CurrentRot)) + (D.y) * (-sin((CurrentRot))), (D.x) * sin((CurrentRot)) + (D.y) * cos((CurrentRot))); // The x of the top right point of the "Other" Rectangle


    // The point which the circle is going to hit (lets call it point T)
    glm::vec2 T(SapphireEngine::ClampFunc(F.x, E.x, current->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(H.y, F.y, current->GetTransform()->Position.value<glm::vec3>()[0]));
    glm::vec2 DiffVector(SapphireEngine::ClampFunc(C.x, A.x, obj->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(B.y, C.y, obj->GetTransform()->Position.value<glm::vec3>()[1]));
    glm::vec2 V(SapphireEngine::ClampFunc(C.x, A.x, T.x), SapphireEngine::ClampFunc(B.y, C.y, T.y));

    // Checking if the length of the vector with points T and the circle's position is less than the radius
    //On the Geogebra file this is the u vector                   //On the Geogebra file this is the v vector
    if (SapphireEngine::LengthVec(current->GetTransform()->Position.value<glm::vec3>(), T) <= SapphireEngine::LengthVec(current->GetTransform()->Position.value<glm::vec3>(), V))
    {
        obj->OnCollision(current);
        current->OnCollision(obj.get());
        if(obj->GetComponent<RigidBody>()->Static.value<bool>()){
            float& mass = current->GetComponent<RigidBody>()->Mass.value<float>();
            // glm::vec3& StartingVelocity = current->GetComponent<RigidBody>()->VelocityLastFrame;
            // glm::vec3 Force = glm::vec3(0,-PhysicsEngine::g.value<float>(),0) -(mass * StartingVelocity / Engine::Get().GetDeltaTime());
            // current->GetComponent<RigidBody>()->Forces.push_back(-Force);
            // float torque = mass * -PhysicsEngine::g.value<float>() * abs(obj->GetTransform()->Position.value<glm::vec3>().x - T.x);
            // float Inertia = mass * (obj->GetTransform()->Position.value<glm::vec3>().x - T.x) * (obj->GetTransform()->Position.value<glm::vec3>().x - T.x);
            // float RotationalAccelaration = torque/Inertia;
            // current->GetTransform()->Rotation.value<glm::vec3>().z = RotationalAccelaration;
        }
        return true;
    }
    return false;

}
bool PhysicsEngine::CirclexCircle(std::shared_ptr<Object> obj, Object* current)
{
    //! Comments here
    // Checking if the length of the vector with points the circles points is less than the sum of the radiuses
    glm::vec2 Distance(current->GetTransform()->Position.value<glm::vec3>().x - obj->GetTransform()->Position.value<glm::vec3>().x, current->GetTransform()->Position.value<glm::vec3>().y - obj->GetTransform()->Position.value<glm::vec3>().y);
    if (SapphireEngine::LengthVec(Distance) < obj->GetTransform()->Size.value<glm::vec3>().x/2 + current->GetTransform()->Size.value<glm::vec3>().x/2) // GetSize().x/2 Basically refers to the radius
    {
        obj->OnCollision(current);
        current->OnCollision(obj.get());
        return true;
//        if(obj->GetTrigger()) return;
//        glm::vec2 P(Distance.x / SapphireEngine::LengthVec(Distance), Distance.y / SapphireEngine::LengthVec(Distance));
//        glm::vec2 L(glm::vec2(obj->GetTransform()->Position.value<glm::vec3>()) + P * (obj->GetTransform()->Size.value<glm::vec3>().x/2));
//        glm::vec2 v(current->GetTransform()->Position.x - L.x, current->GetTransform()->Position.y - L.y);
//        float diff = current->GetTransform()->Size.value<glm::vec3>().x/2 - SapphireEngine::LengthVec(v);
//        glm::vec2 test = P * diff;
//        current->SetPos(current->GetTransform()->Position + glm::vec3(test, 0));
    }
    return false;
}

glm::vec3 PhysicsEngine::Impulse(RigidBody* rb, glm::vec3&& Force)
{
    float& mass = rb->Mass.value<float>();
    glm::vec3 weight = glm::vec3(0,g.value<float>() * mass,0);
    glm::vec3 Fnet = Force + SapphireEngine::VectorSum(rb->Forces) - weight;
    float DeltaTime = 0.2f;
    return glm::vec3(Fnet * DeltaTime) / mass;
}
