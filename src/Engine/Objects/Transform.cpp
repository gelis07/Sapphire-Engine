#include "Transform.h"

Transform::Transform(std::string File, std::string ArgName, unsigned int ArgId,  std::vector<glm::vec3> aPoints, bool LuaComp)
: Component(std::move(File), std::move(ArgName), ArgId, LuaComp), Position("Position", Variables), Rotation("Rotation", Variables), Size("Size", Variables), OriginalPoints(aPoints)
{
    Position.Get() = glm::vec3(0);
    Rotation.Get() = glm::vec3(0);
    Size.Get() = glm::vec3(20,20,0);
    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position.Get());
    Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
    Model = glm::scale(Model, Size.Get());
    for (size_t i = 0; i < OriginalPoints.size(); i++)
    {
        Points.push_back(glm::vec3(Model * glm::vec4(OriginalPoints[i],1.0f)));
    }
    std::function<void()> OnChange = [this]() {
        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position.Get());
        Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
        Model = glm::scale(Model, Size.Get());
        for (size_t i = 0; i < Points.size(); i++)
        {
            Points[i] = glm::vec3(Model * glm::vec4(OriginalPoints[i],1.0f));
        }
    };
    Position.SetOnChangeFunc(OnChange);
    Rotation.SetOnChangeFunc(OnChange);
    Size.SetOnChangeFunc(OnChange);
    Rotation.SliderSpeed = 0.05f;
}

void Transform::Move(const glm::vec3 &translation)
{
    Position.Get() += translation;
    UpdateModel();
    UpdatePoints();    
}

void Transform::Rotate(const float &amount)
{
    Rotation.Get().z += amount;
    UpdateModel();
    UpdatePoints();
}

void Transform::SetPosition(const glm::vec3& NewPosition)
{
    Position.Get() = NewPosition;
    UpdateModel();
    UpdatePoints();
}

void Transform::SetRotation(const float& NewRotation)
{
    Rotation.Get() = glm::vec3(Rotation.Get().x, Rotation.Get().y,NewRotation);
    UpdateModel();
    UpdatePoints();
}

void Transform::SetSize(const glm::vec3& NewSize)
{
    Size.Get() = NewSize;
    UpdateModel();
    UpdatePoints();
}

void Transform::UpdateModel()
{
    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position.Get());
    Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
    Model = glm::scale(Model, Size.Get());
}

void Transform::UpdatePoints()
{
    for (size_t i = 0; i < Points.size(); i++)
    {
        Points[i] = glm:: vec3(Model * glm::vec4(OriginalPoints[i],1.0f));
    }
}
