#include "CollisionDetection.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "UI/Windows.h" 
#include "RunTime/RunTime.h"

SapphireEngine::Float PhysicsEngine::CollisionDetection::g("g", Windows::SettingsVariables);


bool PhysicsEngine::CollisionDetection::CirclexRectangle(Object* obj, Object* current,CollisionData& CD)
{
    CD.Depth = INFINITY;
    std::array<glm::vec2, 4> Obj1Points = ((Shapes::Rectangle*)(current->GetRenderer()->shape.get()))->Points;
    glm::vec2 Obj1Position = glm::vec2(current->GetTransform()->Position.value<glm::vec3>().x, current->GetTransform()->Position.value<glm::vec3>().y);
    glm::vec2 Obj2Position = glm::vec2(obj->GetTransform()->Position.value<glm::vec3>().x, obj->GetTransform()->Position.value<glm::vec3>().y);
    float Obj2Radius = obj->GetTransform()->Size.value<glm::vec3>().x / 2.0f;
    for (size_t i = 0; i < Obj1Points.size(); i++)
    {
        Obj1Points[i] += Obj1Position;
    }
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
    glm::vec2& Cp = Obj1Points[CpIndex];
    float MinObj1 = INFINITY, MaxObj1 = -INFINITY;
    AxisProj = Cp - Obj2Position;
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
    // CD.Depth /= glm::length(CD.Normal);
    // CD.Normal = glm::normalize(CD.Normal);
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
void PhysicsEngine::CollisionDetection::FindPolygonContactPoint(std::shared_ptr<Object> obj, Object *current, glm::vec2& ContactPoint1, glm::vec2& ContactPoint2, int& ContactPointCount)
{
    ContactPoint1 = glm::vec2(0);
    ContactPoint2 = glm::vec2(0);
    ContactPointCount = 0;
    glm::vec2 CurrentPos = glm::vec2(current->GetTransform()->Position.value<glm::vec3>());
    glm::vec2 ObjPos = glm::vec2(obj->GetTransform()->Position.value<glm::vec3>());
    std::array<glm::vec2, 4> CurrentPoints = ((Shapes::Rectangle*)(current->GetRenderer()->shape.get()))->Points;
    std::array<glm::vec2, 4> ObjPoints = ((Shapes::Rectangle*)(obj->GetRenderer()->shape.get()))->Points;
    for (size_t i = 0; i < 4; i++)
    {
        CurrentPoints[i] += CurrentPos;
        ObjPoints[i] += ObjPos;
    }
    

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
        glm::vec2 ObjPoint = ObjPoints[i];
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
glm::vec2 PhysicsEngine::CollisionDetection::FindPolygonCircleContactPoint(const glm::vec2 &CirclePosition, const float &Radius, const glm::vec2 &PolygonPosition, const std::array<glm::vec2, 4> &PolygonPoints)
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
bool PhysicsEngine::CollisionDetection::NearlyEqual(float a, float b)
{
    return abs(a - b) < VerySmallAmount;

}
bool PhysicsEngine::CollisionDetection::NearlyEqual(glm::vec2 &a, glm::vec2 &b)
{
    return (glm::distance(a, b) * glm::distance(a, b)) < VerySmallAmount * VerySmallAmount;
}
int PhysicsEngine::CollisionDetection::FindClosestPointOnPolygon(const glm::vec2 &Position, std::array<glm::vec2, 4> Points)
{
    int result = -1;
    float MinDistance = INFINITY;
    for (size_t i = 0; i < Points.size(); i++)
    {
        glm::vec2& Point = Points[i];

        float Distance = SapphireEngine::LengthVec(glm::vec2(Point - Position));

        if(Distance < MinDistance){
            MinDistance = Distance;
            result = i;
        }
    }
    
    return result;
}
void PhysicsEngine::CollisionDetection::ProjectCircle(const glm::vec2 &Position, float Radius, glm::vec2 Axis, float &o_Min, float &o_Max)
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
    CD.Depth /= glm::length(CD.Normal);
    CD.Normal = glm::normalize(CD.Normal);
    glm::vec2 Direction = Obj2Position - Obj1Position;
    if(glm::dot(Direction, CD.Normal) < 0.0f)
    {
        CD.Normal = -CD.Normal;
    }
    FindPolygonContactPoint(obj, current, CD.ContactPoint1, CD.ContactPoint2, CD.ContactPointCount);
    return true;
}
bool PhysicsEngine::CollisionDetection::CirclexCircle(std::shared_ptr<Object> obj, Object* current, CollisionData& CD)
{
    //! Comments here
    // Checking if the length of the vector with points the circles points is less than the sum of the radiuses
    glm::vec2 DistanceVec(current->GetTransform()->Position.value<glm::vec3>().x - obj->GetTransform()->Position.value<glm::vec3>().x, current->GetTransform()->Position.value<glm::vec3>().y - obj->GetTransform()->Position.value<glm::vec3>().y);
    float Distance = SapphireEngine::LengthVec(DistanceVec);
    float radii = obj->GetTransform()->Size.value<glm::vec3>().x / 2.0f + current->GetTransform()->Size.value<glm::vec3>().x / 2.0f; // GetSize().x/2 Basically refers to the radius
    CD.ContactPoint1 = glm::normalize(DistanceVec) * current->GetTransform()->Size.value<glm::vec3>().x / 2.0f;
    CD.ContactPointCount = 1;
    CD.Normal = glm::normalize(glm::vec2(obj->GetTransform()->Position.value<glm::vec3>() - current->GetTransform()->Position.value<glm::vec3>()));
    CD.Depth = radii - Distance;
    return Distance < radii; 
}

