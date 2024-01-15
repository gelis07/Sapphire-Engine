#include "Transform.h"
#include "Editor/DebugDraw.h"
#define PI 3.14159265359
Transform::Transform(std::string ArgName, std::vector<glm::vec3> aPoints)
: Component(std::move(ArgName)), Position("Position", Variables), Rotation("Rotation", Variables), Size("Size", Variables), OriginalPoints(aPoints)
{
    Position.Get() = glm::vec3(0);
    Rotation.Get() = glm::vec3(0);
    Size.Get() = glm::vec3(1,1,0);
    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position.Get() TOPIXELS);
    Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
    Model = glm::scale(Model, Size.Get() TOPIXELS);
    for (size_t i = 0; i < OriginalPoints.size(); i++)
    {
        Points.push_back(glm::vec3(Model * glm::vec4(OriginalPoints[i],1.0f)));
    }
    std::function<void()> OnChange = [this]() {
        UpdateModel();

    };
    Position.SetOnChangeFunc(OnChange);
    Rotation.SetOnChangeFunc(OnChange);
    Size.SetOnChangeFunc(OnChange);
    Rotation.SliderSpeed = 0.05f;
    Functions["Move"] = MoveLua;
    Functions["Rotate"] = RotateLua;
    Functions["LookAt"] = LookAt;
    Functions["SetPosition"] = SetPositionLua;
    Functions["SetRotation"] = SetRotationLua;
}

Transform::Transform(const Transform &transform)
: Component(std::move(""), std::move("Renderer"), 0, false), Position("Position", Variables), Rotation("Rotation", Variables), Size("Size", Variables), OriginalPoints(transform.GetPoints())
{
    Position.Get() = transform.GetPosition();
    Rotation.Get() = transform.GetRotation();
    Size.Get() = transform.GetSize();
    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, Position.Get());
    Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
    Model = glm::scale(Model, Size.Get());
    for (size_t i = 0; i < OriginalPoints.size(); i++)
    {
        Points.push_back(glm::vec3(Model * glm::vec4(OriginalPoints[i],1.0f)));
    }
    std::function<void()> OnChange = [this]() {
        UpdateModel();
    };
    Position.SetOnChangeFunc(OnChange);
    Rotation.SetOnChangeFunc(OnChange);
    Size.SetOnChangeFunc(OnChange);
    Rotation.SliderSpeed = 0.05f;
    Functions["Move"] = MoveLua;
    Functions["Rotate"] = RotateLua;
    Functions["SetPosition"] = SetPositionLua;
}

void Transform::Move(const glm::vec3 &translation)
{
    SetPosition(Position.Get() += translation);
}

void Transform::Rotate(const float &amount)
{
    SetRotation(Rotation.Get().z += amount);
}

void Transform::SetPosition(const glm::vec3& NewPosition)
{
    Position.Get() = NewPosition;
    UpdateModel();
    UpdatePoints();
}

glm::vec3 Transform::GetWorldPositon() const
{
    if(Parent != nullptr){
        return Position.Get() + Parent->GetPosition();
    }
    return Position.Get();
}

void Transform::SetRotation(const float &NewRotation)
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

int Transform::MoveLua(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Transform* transform = static_cast<Transform*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    transform->Move(glm::vec3(x,y,0));
    return 0;
}

int Transform::LookAt(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Transform* transform = static_cast<Transform*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    glm::vec2 diffVec = glm::vec2(x,y) - glm::vec2(transform->GetWorldPositon());
    float angle = glm::acos(diffVec.y/glm::length(diffVec));
    if(diffVec.x > 0 && diffVec.y > 0)
        angle *= -1;
    else if(diffVec.x > 0 && diffVec.y < 0)
        angle *= -1;

    // transform->SetRotation(angle);
    lua_pushnumber(L, angle);
    return 1;
}
int Transform::SetPositionLua(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Transform* transform = static_cast<Transform*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    transform->SetPosition(glm::vec3(x,y,0));
    return 0;
}

int Transform::SetRotationLua(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Transform* transform = static_cast<Transform*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float angle = (float)luaL_checknumber(L, 2);
    transform->SetRotation(angle);
    return 0;
}

int Transform::RotateLua(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Transform* transform = static_cast<Transform*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float amount = (float)luaL_checknumber(L, 2);
    transform->Rotate(amount);
    return 0;
}

void Transform::UpdateModel()
{
    if(Parent == nullptr){
        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, Position.Get() TOPIXELS);
        Model = glm::rotate(Model, Rotation.Get().z, glm::vec3(0,0,1));
        Model = glm::scale(Model, Size.Get() TOPIXELS);
        for (auto &&child : childrenTransforms)
        {
            child->Model = glm::mat4(1.0f);
            child->Model = glm::translate(child->Model, (Position.Get() + child->GetPosition()) TOPIXELS);
            child->Model = glm::rotate(child->Model, Rotation.Get().z + child->GetRotation().z, glm::vec3(0,0,1));
            child->Model = glm::scale(child->Model, child->GetSize() TOPIXELS);
        }
    }else{
        Parent->UpdateModel();
    }
}

void Transform::UpdatePoints()
{
    for (size_t i = 0; i < Points.size(); i++)
    {
        Points[i] = glm::vec3(Model * glm::vec4(OriginalPoints[i],1.0f)) TOUNITS;
    }
}
