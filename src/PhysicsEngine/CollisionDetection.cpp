#include "CollisionDetection.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "UI/Windows.h" 
#include "RunTime/RunTime.h"

SapphireEngine::Float PhysicsEngine::CollisionDetection::g("g", Windows::SettingsVariables);


bool PhysicsEngine::CollisionDetection::CirclexRectangle(std::shared_ptr<Object> obj, Object* current)
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
//Thanks to the video by https://www.youtube.com/watch?v=5gDC1GU3Ivg&list=PLSlpr6o9vURwq3oxVZSimY8iC-cdd3kIs&index=22

void PhysicsEngine::CollisionDetection::FindContactPoint(std::shared_ptr<Object> obj, Object *current, glm::vec2& ContactPoint1, glm::vec2& ContactPoint2, int& ContactPointCount)
{
    ContactPoint1 = glm::vec2(0);
    ContactPoint2 = glm::vec2(0);
    ContactPointCount = 0;
    std::array<glm::vec2, 4> CurrentPoints = ((Shapes::Rectangle*)(current->GetRenderer()->shape.get()))->Points;
    std::array<glm::vec2, 4> ObjPoints = ((Shapes::Rectangle*)(obj->GetRenderer()->shape.get()))->Points;

    float minDistanceSquared = INFINITY;

    for (size_t i = 0; i < CurrentPoints.size(); i++)
    {
        glm::vec2 CurrentPoint = CurrentPoints[i];
        for (size_t j = 0; j < ObjPoints.size(); j++)
        {
            glm::vec2 ObjPoint = ObjPoints[j];
            glm::vec2 ObjNextPoint = ObjPoints[(j+1) % ObjPoints.size()]; // Making sure its inside the points array
            float DistanceSquared = 0.0f;
            glm::vec2 Cp = glm::vec2(0);
            PointSegmentDistance(CurrentPoint, ObjPoint, ObjNextPoint, DistanceSquared, Cp);

            if(DistanceSquared == minDistanceSquared){
                if(Cp != ContactPoint1)
                {
                    ContactPoint2 = Cp;
                    ContactPointCount = 2;
                }
            }
            else if(DistanceSquared < minDistanceSquared){
                minDistanceSquared = DistanceSquared;
                ContactPointCount = 1;
                ContactPoint1 = Cp;
            }
        }
        
    }
    for (size_t i = 0; i < ObjPoints.size(); i++)
    {
        glm::vec2 ObjPoint = CurrentPoints[i];
        for (size_t j = 0; j < CurrentPoints.size(); j++)
        {
            glm::vec2 CurrentPoints = ObjPoints[j];
            glm::vec2 ObjNextPoint = ObjPoints[(j+1) % ObjPoints.size()]; // Making sure its inside the points array
            float DistanceSquared = 0.0f;
            glm::vec2 Cp;
            PointSegmentDistance(ObjPoint, CurrentPoints, ObjNextPoint, DistanceSquared, Cp);

            if(DistanceSquared == minDistanceSquared){
                if(Cp != ContactPoint1)
                {
                    ContactPoint2 = Cp;
                    ContactPointCount = 2;
                }
            }
            else if(DistanceSquared < minDistanceSquared){
                minDistanceSquared = DistanceSquared;
                ContactPointCount = 1;
                ContactPoint1 = Cp;
            }
        }
        
    }
    
}
glm::vec2 PhysicsEngine::CollisionDetection::FindArithmeticMean(std::array<glm::vec2, 4> &Vertices)
{
    glm::vec2 Sum;

    for (size_t i = 0; i < Vertices.size(); i++)
    {
        Sum += Vertices[i];
    }
    Sum /= Vertices.size();
    return Sum;
}
void PhysicsEngine::CollisionDetection::PointSegmentDistance(glm::vec2 p, glm::vec2 a, glm::vec2 b, float &distanceSquared, glm::vec2 &cp)
{
    glm::vec2 ab = b - a;
    glm::vec2 ap = p - a;

    float Projection = glm::dot(ap, ab);
    float abLengthSquared = ab.x * ab.x + ab.y * ab.y;
    float d = Projection / abLengthSquared;
    if(d <= 0.0f)
        cp = a;
    else if(d >= 1.0f)
        cp = b;
    else
        cp = a + ab * d;
    
    float dx = p.x - cp.x;
    float dy = p.y - cp.y;
    distanceSquared = dx * dx + dy * dy;
}

// Would like to thank Javidx9 for the amazing video on implementing SAT (Seperated Axis Theorem) with c++! https://www.youtube.com/watch?v=7Ik2vowGcU0
bool PhysicsEngine::CollisionDetection::RectanglexRectangle(std::shared_ptr<Object> obj, Object *current, CollisionData& CD)
{
    Object* Obj1 = obj.get();
    Object* Obj2 = current;

    CD.Normal = glm::vec2(0);
    CD.Depth = INFINITY;
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
        //"Inverting" the shapes so both objects are tested on eachother.
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
            AxisProj = glm::normalize(AxisProj);
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
            float AxisDepth = std::min(MaxObj2 - MinObj1, MaxObj1 - MinObj2);
            if(AxisDepth < CD.Depth){
                CD.Depth = AxisDepth;
                CD.Normal = AxisProj;
            }
        }
    }

    glm::vec2 Direction = Obj2Position - Obj1Position;
    if(glm::dot(Direction, CD.Normal) < 0.0f)
    {
        CD.Normal = -CD.Normal;
    }
    FindContactPoint(obj, current, CD.ContactPoint1, CD.ContactPoint2, CD.ContactPointCount);
    return true;
}
bool PhysicsEngine::CollisionDetection::CirclexCircle(std::shared_ptr<Object> obj, Object* current)
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

