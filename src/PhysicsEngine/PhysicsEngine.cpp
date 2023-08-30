#include "PhysicsEngine.h"
#include "Objects/Objects.h"

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
    // These points are linked to the geogebra files!
    float Hy = obj->GetTransform()->Position.value<glm::vec3>().y - obj->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the bottom Left point of the "Other" Rectangle
    float Fy = obj->GetTransform()->Position.value<glm::vec3>().y + obj->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the top Left point of the "Other" Rectangle
    float Fx = obj->GetTransform()->Position.value<glm::vec3>().x - obj->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top Left point of the "Other" Rectangle
    float Ex = obj->GetTransform()->Position.value<glm::vec3>().x + obj->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top right point of the "Other" Rectangle

    float By = current->GetTransform()->Position.value<glm::vec3>().y - current->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the bottom Left point of the Selected Rectangle
    float Cy = current->GetTransform()->Position.value<glm::vec3>().y + current->GetTransform()->Size.value<glm::vec3>().y / 2; // The y of the top Left point of the Selected Rectangle
    float Cx = current->GetTransform()->Position.value<glm::vec3>().x - current->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top Left point of the Selected Rectangle
    float Ax = current->GetTransform()->Position.value<glm::vec3>().x + current->GetTransform()->Size.value<glm::vec3>().x / 2; // The x of the top right point of the Selected Rectangle

    // The point which the circle is going to hit (lets call it point T)
    glm::vec2 T(SapphireEngine::ClampFunc(Fx, Ex, current->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(Hy, Fy, current->GetTransform()->Position.value<glm::vec3>()[0]));
    glm::vec2 DiffVector(SapphireEngine::ClampFunc(Cx, Ax, obj->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(By, Cy, obj->GetTransform()->Position.value<glm::vec3>()[1]));
    glm::vec2 V(SapphireEngine::ClampFunc(Cx, Ax, T.x), SapphireEngine::ClampFunc(By, Cy, T.y));

    // Checking if the length of the vector with points T and the circle's position is less than the radius
    //On the Geogebra file this is the u vector                   //On the Geogebra file this is the v vector
    if (SapphireEngine::LengthVec(current->GetTransform()->Position.value<glm::vec3>(), T) <= SapphireEngine::LengthVec(current->GetTransform()->Position.value<glm::vec3>(), V))
    {
        obj->OnCollision(current);
        current->OnCollision(obj.get());
        return true;

//        if(obj->GetTrigger()) return;
//        if(obj->IsStatic){
            // current->Force()
//        }
        //Calculating the difference so we can position the other rectangle accordingly
        // float diff = SapphireEngine::LengthVec(obj->GetTransform()->Position, DiffVector) - SapphireEngine::LengthVec(obj->GetTransform()->Position, T);
        // glm::vec2 NormalizedT((DiffVector.x - current->GetTransform()->Position.x) / SapphireEngine::LengthVec(DiffVector, current->GetTransform()->Position), (DiffVector.y - current->GetTransform()->Position.y) / SapphireEngine::LengthVec(DiffVector, current->GetTransform()->Position));

        // glm::vec3 VectorDiff(NormalizedT * diff, 0.0f);
        // if (isnan(NormalizedT.x))
        // {
        //     VectorDiff = glm::vec3(0, 0, 0);
        // }
        // current->SetPos(current->GetTransform()->Position + VectorDiff);
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
    glm::vec3 weight = glm::vec3(0,g * mass,0);
    glm::vec3 Fnet = Force + SapphireEngine::VectorSum(rb->Forces) - weight;
    float DeltaTime = 0.2f;
    return glm::vec3(Fnet * DeltaTime) / mass;
}
