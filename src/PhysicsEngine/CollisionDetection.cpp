#include "CollisionDetection.h"
#include "PhysicsEngine/RigidBody.h"
#include "Engine/Engine.h"
#include "Editor.h"
SapphireEngine::Float SapphirePhysics::CollisionDetection::g("g", Engine::SettingsVariables);


bool SapphirePhysics::CollisionDetection::CirclexRectangle(Object* obj, Object* current,CollisionData& CD)
{
    CD.Depth = INFINITY;
    std::array<glm::vec3, 4> Obj1Points = {current->GetComponent<Transform>()->GetPoints()[0], current->GetComponent<Transform>()->GetPoints()[1],
    current->GetComponent<Transform>()->GetPoints()[2], current->GetComponent<Transform>()->GetPoints()[3]};
    glm::vec2 Obj1Position = glm::vec2(current->GetComponent<Transform>()->GetPosition().x, current->GetComponent<Transform>()->GetPosition().y);
    glm::vec2 Obj2Position = glm::vec2(obj->GetComponent<Transform>()->GetPosition().x, obj->GetComponent<Transform>()->GetPosition().y);
    float Obj2Radius = obj->GetComponent<Transform>()->GetSize().x / 2.0f;

    float AxisDepth = 0;
    glm::vec2 AxisProj;
    for (size_t i = 0; i < Obj1Points.size(); i++)
    {
        //Ni = Next Index. Making sure to be in range
        int Ni = (i + 1) % Obj1Points.size();
        //Perpendicular vector to the currently selected axis.
        AxisProj = glm::vec2(-(Obj1Points[Ni].y - Obj1Points[i].y), Obj1Points[Ni].x - Obj1Points[i].x);
        AxisProj = glm::normalize(AxisProj);
        float MinObj1 = INFINITY, MaxObj1 = -INFINITY;
        for (size_t j = 0; j < Obj1Points.size(); j++)
        {
            float DotProduct = Obj1Points[j].x * AxisProj.x + Obj1Points[j].y * AxisProj.y;
            MinObj1 = std::min(MinObj1, DotProduct);
            MaxObj1 = std::max(MaxObj1, DotProduct);
        }
        float MinObj2 = INFINITY,MaxObj2 = -INFINITY;
        ProjectCircle(Obj2Position, Obj2Radius, AxisProj, MinObj2, MaxObj2);
        if(MinObj1 >= MaxObj2 || MinObj2 >= MaxObj1){
            return false;
        }
        AxisDepth = std::min(MaxObj2 - MinObj1, MaxObj1 - MinObj2);
        if(AxisDepth < CD.Depth){
            CD.Depth = AxisDepth;
            CD.Normal = AxisProj;
        }
    }

    int CpIndex = FindClosestPointOnPolygon(Obj2Position, Obj1Points);
    glm::vec3& Cp = Obj1Points[CpIndex];
    float MinObj1 = INFINITY, MaxObj1 = -INFINITY;
    AxisProj = glm::vec2(Cp) - Obj2Position;
    AxisProj = glm::normalize(AxisProj);
    for (size_t j = 0; j < Obj1Points.size(); j++)
    {
        float DotProduct = Obj1Points[j].x * AxisProj.x + Obj1Points[j].y * AxisProj.y;
        MinObj1 = std::min(MinObj1, DotProduct);
        MaxObj1 = std::max(MaxObj1, DotProduct);
    }
    float MinObj2 = INFINITY,MaxObj2 = -INFINITY;
    ProjectCircle(Obj2Position, Obj2Radius, AxisProj, MinObj2, MaxObj2);
    if(MinObj1 >= MaxObj2 || MinObj2 >= MaxObj1){
        return false;
    }
    AxisDepth = std::min(MaxObj2 - MinObj1, MaxObj1 - MinObj2);
    if(AxisDepth < CD.Depth){
        CD.Depth = AxisDepth;
        CD.Normal = AxisProj;
    }

    glm::vec2 Direction = Obj1Position - Obj2Position;
    if(glm::dot(Direction, CD.Normal) < 0.0f)
    {
        CD.Normal = -CD.Normal;
    }
    CD.ContactPoint1 = FindPolygonCircleContactPoint(Obj2Position, Obj2Radius, Obj1Position, Obj1Points);
    CD.ContactPointCount = 1;
    return true;
}

//Thanks to the video by https://www.youtube.com/watch?v=5gDC1GU3Ivg&list=PLSlpr6o9vURwq3oxVZSimY8iC-cdd3kIs&index=22
void SapphirePhysics::CollisionDetection::FindPolygonContactPoint(SapphirePhysics::RigidBody* bodyA, SapphirePhysics::RigidBody* bodyB, 
glm::vec2& ContactPoint1, glm::vec2& ContactPoint2, int& ContactPointCount)
{
    ContactPoint1 = glm::vec2(0);
    ContactPoint2 = glm::vec2(0);
    ContactPointCount = 0;
    glm::vec2 BodyAPos = glm::vec2(bodyA->transform->GetPosition());
    glm::vec2 BodyBPos = glm::vec2(bodyB->transform->GetPosition());
    std::array<glm::vec3, 4> CurrentPoints = {bodyA->transform->GetPoints()[0], bodyA->transform->GetPoints()[1],
    bodyA->transform->GetPoints()[2], bodyA->transform->GetPoints()[3]};
    std::array<glm::vec3, 4> ObjPoints = {bodyB->transform->GetPoints()[0], bodyB->transform->GetPoints()[1],
    bodyB->transform->GetPoints()[2], bodyB->transform->GetPoints()[3]};

    float minDistanceSquared = INFINITY;

    for (size_t i = 0; i < CurrentPoints.size(); i++)
    {
        const glm::vec2& CurrentPoint = CurrentPoints[i];
        for (size_t j = 0; j < ObjPoints.size(); j++)
        {
            glm::vec2 ObjPoint = ObjPoints[j];
            glm::vec2 ObjNextPoint = ObjPoints[(j+1) % ObjPoints.size()]; // Making sure its inside the points array
            float DistanceSquared = 0.0f;
            glm::vec2 Cp = glm::vec2(0);
            PointSegmentDistance(CurrentPoint, ObjPoint, ObjNextPoint, DistanceSquared, Cp);

            if(NearlyEqual(DistanceSquared, minDistanceSquared)){
                if(!NearlyEqual(Cp, ContactPoint1))
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
        const glm::vec2& ObjPoint = ObjPoints[i];
        for (size_t j = 0; j < CurrentPoints.size(); j++)
        {
            glm::vec2 CurrentPoint = CurrentPoints[j];
            glm::vec2 CurrentNextPoint = CurrentPoints[(j+1) % CurrentPoints.size()]; // Making sure its inside the points array
            float DistanceSquared = 0.0f;
            glm::vec2 Cp;
            PointSegmentDistance(ObjPoint, CurrentPoint, CurrentNextPoint, DistanceSquared, Cp);

            if(NearlyEqual(DistanceSquared, minDistanceSquared)){
                if(!NearlyEqual(Cp, ContactPoint1))
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
glm::vec2 SapphirePhysics::CollisionDetection::FindPolygonCircleContactPoint(const glm::vec2 &CirclePosition, const float &Radius, const glm::vec2 &PolygonPosition, const std::array<glm::vec3, 4> &PolygonPoints)
{
    float MinDistanceSquared = INFINITY;
    glm::vec2 ContactPoint = glm::vec2(0);
    for (size_t i = 0; i < PolygonPoints.size(); i++)
    {
        const glm::vec2& Point = PolygonPoints[i];
        const glm::vec2& NextPoint = PolygonPoints[(i + 1) % PolygonPoints.size()];

        float DistanceSquared = 0;
        glm::vec2 NewContactPoint;
        PointSegmentDistance(CirclePosition, Point, NextPoint, DistanceSquared, NewContactPoint);

        if(DistanceSquared < MinDistanceSquared){
            MinDistanceSquared = DistanceSquared;
            ContactPoint = NewContactPoint;
        }
    }

    return ContactPoint;
}
constexpr float VerySmallAmount = 0.0005f;
bool SapphirePhysics::CollisionDetection::NearlyEqual(float a, float b)
{
    return abs(a - b) < VerySmallAmount;

}
bool SapphirePhysics::CollisionDetection::NearlyEqual(glm::vec2 &a, glm::vec2 &b)
{
    return (glm::distance(a, b) * glm::distance(a, b)) < VerySmallAmount * VerySmallAmount;
}
int SapphirePhysics::CollisionDetection::FindClosestPointOnPolygon(const glm::vec2 &Position, std::array<glm::vec3, 4> Points)
{
    int result = -1;
    float MinDistance = INFINITY;
    for (size_t i = 0; i < Points.size(); i++)
    {
        glm::vec3& Point = Points[i];

        float Distance = SapphireEngine::LengthVec(glm::vec2(glm::vec2(Point) - Position));

        if(Distance < MinDistance){
            MinDistance = Distance;
            result = i;
        }
    }
    
    return result;
}
void SapphirePhysics::CollisionDetection::ProjectCircle(const glm::vec2 &Position, float Radius, glm::vec2 Axis, float &o_Min, float &o_Max)
{
    glm::vec2 Direction = glm::normalize(Axis);
    glm::vec2 DirectionAndRadius = Direction * Radius;

    glm::vec2 Point1 = Position + DirectionAndRadius;
    glm::vec2 Point2 = Position - DirectionAndRadius;

    o_Min = glm::dot(Point1, Axis);
    o_Max = glm::dot(Point2, Axis);

    if(o_Min > o_Max){
        float t = o_Min;
        o_Min = o_Max;
        o_Max = t;
    }
}
glm::vec2 SapphirePhysics::CollisionDetection::FindArithmeticMean(std::array<glm::vec3, 4> &Vertices)
{
    glm::vec3 Sum;

    for (size_t i = 0; i < Vertices.size(); i++)
    {
        Sum += Vertices[i];
    }
    Sum /= Vertices.size();
    return glm::vec2(Sum);
}
void SapphirePhysics::CollisionDetection::PointSegmentDistance(glm::vec2 p, glm::vec2 a, glm::vec2 b, float &distanceSquared, glm::vec2 &cp)
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
bool SapphirePhysics::CollisionDetection::RectanglexRectangle(SapphirePhysics::RigidBody* bodyA, SapphirePhysics::RigidBody* bodyB, CollisionData& CD)
{
    CD.Normal = glm::vec2(0);
    CD.Depth = INFINITY;
    std::array<glm::vec3, 4> Obj1Points = {bodyA->transform->GetPoints()[0], bodyA->transform->GetPoints()[1],
    bodyA->transform->GetPoints()[2], bodyA->transform->GetPoints()[3]};
    std::array<glm::vec3, 4> Obj2Points = {bodyB->transform->GetPoints()[0], bodyB->transform->GetPoints()[1],
    bodyB->transform->GetPoints()[2], bodyB->transform->GetPoints()[3]};
    glm::vec2 Obj1Position = glm::vec2(bodyA->transform->GetPosition().x, bodyA->transform->GetPosition().y);
    glm::vec2 Obj2Position = glm::vec2(bodyB->transform->GetPosition().x, bodyB->transform->GetPosition().y);
    for (size_t shape = 0; shape < 2; shape++)
    {
        //"Inverting" the shapes so both objects are tested on eachother.
        if(shape == 1){
            //I need a copy because if I do this:
            /*
                bodyA = bodyB
                bodyB = bodyA
                then bodyB would be bodyB so
                BodyA = bodyB
                BodyB = BodyB
                but with that method the bodies are succesfully swaped.
            */
            RigidBody* bodyAcopy = bodyA;
            bodyA = bodyB;
            bodyB = bodyAcopy;

            Obj1Points = {bodyA->transform->GetPoints()[0], bodyA->transform->GetPoints()[1],
            bodyA->transform->GetPoints()[2], bodyA->transform->GetPoints()[3]};
            Obj2Points = {bodyB->transform->GetPoints()[0], bodyB->transform->GetPoints()[1],
            bodyB->transform->GetPoints()[2], bodyB->transform->GetPoints()[3]};
            Obj1Position = glm::vec2(bodyA->transform->GetPosition().x, bodyA->transform->GetPosition().y);
            Obj2Position = glm::vec2(bodyB->transform->GetPosition().x, bodyB->transform->GetPosition().y);
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
    CD.Depth /= glm::length(CD.Normal);
    CD.Normal = glm::normalize(CD.Normal);
    glm::vec2 Direction = Obj2Position - Obj1Position;
    if(glm::dot(Direction, CD.Normal) < 0.0f)
    {
        CD.Normal = -CD.Normal;
    }
    FindPolygonContactPoint(bodyB, bodyA, CD.ContactPoint1, CD.ContactPoint2, CD.ContactPointCount);
    return true;
}
bool SapphirePhysics::CollisionDetection::CirclexCircle(SapphirePhysics::RigidBody* obj, SapphirePhysics::RigidBody* current, CollisionData& CD)
{
    // Checking if the length of the vector with points the circles points is less than the sum of the radiuses
    glm::vec2 DistanceVec(current->transform->GetPosition().x - obj->transform->GetPosition().x, current->transform->GetPosition().y - obj->transform->GetPosition().y);
    float Distance = SapphireEngine::LengthVec(DistanceVec);
    float radii = obj->transform->GetSize().x / 2.0f + current->transform->GetSize().x / 2.0f; // GetSize().x/2 Basically refers to the radius
    CD.ContactPoint1 = glm::normalize(DistanceVec) * current->transform->GetSize().x / 2.0f;
    CD.ContactPointCount = 1;
    CD.Normal = glm::normalize(glm::vec2(obj->transform->GetPosition() - current->transform->GetPosition()));
    CD.Depth = radii - Distance;
    return Distance < radii; 
}

