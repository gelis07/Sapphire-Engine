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
    }
    return false;

}
//Would like to thank Javidx9 for the amazing video on implementing SAT (Seperated Axis Theorem) with c++! https://www.youtube.com/watch?v=7Ik2vowGcU0
bool PhysicsEngine::RectanglexRectangle(std::shared_ptr<Object> obj, Object* current)
{
    Object* Obj1 = obj.get();
    Object* Obj2 = current;

    std::array<glm::vec2, 4> Obj1Points = ((Shapes::Rectangle*)(Obj1->GetRenderer()->shape.get()))->Points;
    std::array<glm::vec2, 4> Obj2Points = ((Shapes::Rectangle*)(Obj2->GetRenderer()->shape.get()))->Points;
    glm::vec2 Obj1Position = glm::vec2(Obj1->GetTransform()->Position.value<glm::vec3>().x, Obj1->GetTransform()->Position.value<glm::vec3>().y);
    glm::vec2 Obj2Position = glm::vec2(Obj2->GetTransform()->Position.value<glm::vec3>().x, Obj2->GetTransform()->Position.value<glm::vec3>().y);
    for (size_t i = 0; i < Obj1Points.size(); i++)
    {
        Obj1Points[i] += Obj1Position;
    }
    for (size_t i = 0; i < Obj2Points.size(); i++)
    {
        Obj2Points[i] += Obj2Position;
    }
    for (size_t shape = 0; shape < 2; shape++)
    {
        //Testing both shapes
        if(shape == 1){
            Obj1 = current;
            Obj2 = obj.get();

            Obj1Points = ((Shapes::Rectangle*)(Obj1->GetRenderer()->shape.get()))->Points;
            Obj2Points = ((Shapes::Rectangle*)(Obj2->GetRenderer()->shape.get()))->Points;
            Obj1Position = glm::vec2(Obj1->GetTransform()->Position.value<glm::vec3>().x, Obj1->GetTransform()->Position.value<glm::vec3>().y);
            Obj2Position = glm::vec2(Obj2->GetTransform()->Position.value<glm::vec3>().x, Obj2->GetTransform()->Position.value<glm::vec3>().y);
            for (size_t i = 0; i < Obj1Points.size(); i++)
            {
                Obj1Points[i] += Obj1Position;
            }
            for (size_t i = 0; i < Obj2Points.size(); i++)
            {
                Obj2Points[i] += Obj2Position;
            }
        }
        for (size_t i = 0; i < Obj1Points.size(); i++)
        {
            //Ni = Next Index. Making sure to be in range
            int Ni = (i + 1) % Obj1Points.size();
            //Perpendicular vector to the currently selected axis.
            glm::vec2 AxisProj = glm::vec2(-(Obj1Points[Ni].y - Obj1Points[i].y), Obj1Points[Ni].x - Obj1Points[i].x);
            
            float MinObj1 = INFINITY, MaxObj1 = -INFINITY;
            for (size_t j = 0; j < Obj1Points.size(); j++)
            {
                float DotProduct = Obj1Points[j].x * AxisProj.x + Obj1Points[j].y * AxisProj.y;
                MinObj1 = std::min(MinObj1, DotProduct);
                MaxObj1 = std::max(MaxObj1, DotProduct);
            }

            float MinObj2 = INFINITY, MaxObj2 = -INFINITY;
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
    }
    glm::vec2 T(SapphireEngine::ClampFunc(Obj1Points[1].x, Obj1Points[0].x, current->GetTransform()->Position.value<glm::vec3>()[0]), SapphireEngine::ClampFunc(Obj1Points[3].y, Obj1Points[2].y, current->GetTransform()->Position.value<glm::vec3>()[0]));
    obj->OnCollision(current);
    current->OnCollision(obj.get());
    if(obj->GetComponent<RigidBody>()->Static.value<bool>()){
        current->GetComponent<RigidBody>()->StartingVelocity = -current->GetComponent<RigidBody>()->e.value<float>() * current->GetComponent<RigidBody>()->Velocity.value<glm::vec3>().y > 1.5f ? -current->GetComponent<RigidBody>()->e.value<float>() * current->GetComponent<RigidBody>()->Velocity.value<glm::vec3>() : glm::vec3(0);
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
