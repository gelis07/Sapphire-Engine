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
    Object* Obj1 = obj.get();
    Object* Obj2 = current;

    std::array<glm::vec2, 4> Obj1Points = ((Shapes::Rectangle*)(Obj1->GetRenderer()->shape.get()))->Points;
    std::array<glm::vec2, 4> Obj2Points = ((Shapes::Rectangle*)(Obj2->GetRenderer()->shape.get()))->Points;

    for (auto& Point : Obj1Points)
    {
        Point += glm::vec2(Obj1->GetTransform()->Position.value<glm::vec3>().x, Obj1->GetTransform()->Position.value<glm::vec3>().y);
    }
    for (auto& Point : Obj2Points)
    {
        Point += glm::vec2(Obj2->GetTransform()->Position.value<glm::vec3>().x, Obj2->GetTransform()->Position.value<glm::vec3>().y);
    }
    

    for (size_t i = 0; i < Obj1Points.size(); i++)
    {
        //Ni = Next Index Making sure to be in range
        int Ni = (i + 1) % Obj1Points.size();
        glm::vec2 AxisProj = glm::vec2(-(Obj1Points[Ni].y - Obj1Points[i].y), Obj1Points[Ni].x - Obj1Points[i].x);
        
        float MinObj1 = INFINITY; float MaxObj1 = -INFINITY;
        for (size_t j = 0; j < Obj1Points.size(); j++)
        {
            float DotProduct = Obj1Points[j].x * AxisProj.x + Obj1Points[j].y * AxisProj.y;
            MinObj1 = std::min(MinObj1, DotProduct);
            MaxObj1 = std::max(MaxObj1, DotProduct);
        }

        float MinObj2 = INFINITY; float MaxObj2 = -INFINITY;
        for (size_t j = 0; j < Obj2Points.size(); j++)
        {
            float DotProduct = Obj2Points[j].x * AxisProj.x + Obj2Points[j].y * AxisProj.y;
            MinObj2 = std::min(MinObj2, DotProduct);
            MaxObj2 = std::max(MaxObj2, DotProduct);
        }
        if(!(MaxObj2 >= MinObj1 && MaxObj1 >= MinObj2)){
            return false;
        }
    }
    
    
    // glm::vec2 T(SapphireEngine::ClampFunc(Obj1Points[0].x, Obj1Points[1].x, current->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(Obj1Points[2].y, Obj1Points[3].y, current->GetTransform()->Position.value<glm::vec3>()[0]));


    // The point which the circle is going to hit (lets call it point T)

    // Checking if the length of the vector with points T and the circle's position is less than the radius
    //On the Geogebra file this is the u vector                   //On the Geogebra file this is the v vector
    obj->OnCollision(current);
    current->OnCollision(obj.get());
    if(obj->GetComponent<RigidBody>()->Static.value<bool>()){
        // float& mass = current->GetComponent<RigidBody>()->Mass.value<float>();
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
