#include "Variables.h"

void SapphireEngine::Float::RenderGUI(){
    ImGui::InputFloat(Name.c_str(), std::any_cast<float>(&data));
}

void SapphireEngine::Float::Save(nlohmann::json &JSON)
{
    JSON[Name] = {typeid(SapphireEngine::Float).hash_code(), std::any_cast<float>(data)};
}

void SapphireEngine::Float::SendToLua(lua_State *L)
{
    lua_pushnumber(L, std::any_cast<float>(data));
}

void SapphireEngine::Float::GetFromLua(lua_State *L)
{
    data = lua_tonumber(L, -1);
}

void SapphireEngine::Float::Load(const nlohmann::json &jsonArray)
{
    data = jsonArray[1].get<float>();
}

void SapphireEngine::Bool::RenderGUI()
{
    ImGui::Checkbox(Name.c_str(), std::any_cast<bool>(&data));
}

void SapphireEngine::Bool::Save(nlohmann::json &JSON)
{
    JSON[Name] = {typeid(SapphireEngine::Bool).hash_code(), std::any_cast<bool>(data)};
}

void SapphireEngine::Bool::SendToLua(lua_State *L)
{
    lua_pushboolean(L, std::any_cast<bool>(data));
}

void SapphireEngine::Bool::GetFromLua(lua_State *L)
{
    data = lua_toboolean(L, -1);
}

void SapphireEngine::Bool::Load(const nlohmann::json &jsonArray)
{
    data = jsonArray[1].get<bool>();
}

void SapphireEngine::String::RenderGUI()
{
    ImGui::InputText(Name.c_str(), std::any_cast<std::string>(&data));
}

void SapphireEngine::String::Save(nlohmann::json &JSON)
{
    JSON[Name] = {typeid(SapphireEngine::String).hash_code(), std::any_cast<std::string>(data)};
}

void SapphireEngine::String::SendToLua(lua_State *L)
{
    lua_pushstring(L, std::any_cast<std::string>(data).c_str());
}

void SapphireEngine::String::GetFromLua(lua_State *L)
{
    data = std::string(lua_tostring(L, -1));
}

void SapphireEngine::String::Load(const nlohmann::json &jsonArray)
{
    data = jsonArray[1].get<std::string>();
}

void SapphireEngine::Vec2::RenderGUI()
{
    ImGui::InputFloat2(Name.c_str(), &(*std::any_cast<glm::vec2>(&data))[0]);
}

void SapphireEngine::Vec2::Save(nlohmann::json &JSON)
{
    nlohmann::json Vector;
    glm::vec2& DataVector = std::any_cast<glm::vec2&>(data);
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    JSON[Name] = {typeid(SapphireEngine::Vec2).hash_code(), Vector};
}

void SapphireEngine::Vec2::SendToLua(lua_State *L)
{
    glm::vec2& vecValue = std::any_cast<glm::vec2&>(data);
    lua_newtable(L);
    lua_pushnumber(L, vecValue.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vecValue.y);
    lua_setfield(L, -2, "y");
}

void SapphireEngine::Vec2::GetFromLua(lua_State *L)
{
    glm::vec2& vecValue = std::any_cast<glm::vec2&>(data);
    lua_getfield (L, -1, "x");
    vecValue.x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "y");
    vecValue.y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    data = vecValue;
}

void SapphireEngine::Vec2::Load(const nlohmann::json &jsonArray)
{
    glm::vec2 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    data = vector;
}

void SapphireEngine::Vec3::RenderGUI()
{
    ImGui::InputFloat3(Name.c_str(), &(*std::any_cast<glm::vec3>(&data))[0]);
}

void SapphireEngine::Vec3::Save(nlohmann::json &JSON)
{
    nlohmann::json Vector;
    glm::vec3& DataVector = std::any_cast<glm::vec3&>(data);
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    Vector["z"] = DataVector.z;
    JSON[Name] = {typeid(SapphireEngine::Vec3).hash_code(), Vector};
}

void SapphireEngine::Vec3::SendToLua(lua_State *L)
{
    glm::vec3& vecValue = std::any_cast<glm::vec3&>(data);
    lua_newtable(L);
    lua_pushnumber(L, vecValue.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vecValue.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, vecValue.z);
    lua_setfield(L, -2, "z");
}

void SapphireEngine::Vec3::GetFromLua(lua_State *L)
{
    glm::vec3& vecValue = std::any_cast<glm::vec3&>(data);
    lua_getfield (L, -1, "x");
    vecValue.x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "y");
    vecValue.y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "z");
    vecValue.z = lua_tonumber(L, -1);
    lua_pop(L, 1);
    data = vecValue;
}

void SapphireEngine::Vec3::Load(const nlohmann::json &jsonArray)
{
    glm::vec3 Vector;
    Vector.x = jsonArray[1]["x"].get<float>();
    Vector.y = jsonArray[1]["y"].get<float>();
    Vector.z = jsonArray[1]["z"].get<float>();
    data = Vector;
}

void SapphireEngine::Vec4::RenderGUI()
{
    ImGui::InputFloat4(Name.c_str(), &(*std::any_cast<glm::vec4>(&data))[0]);
}

void SapphireEngine::Vec4::Save(nlohmann::json &JSON)
{
    nlohmann::json Vector;
    glm::vec4& DataVector = std::any_cast<glm::vec4&>(data);
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    Vector["z"] = DataVector.z;
    Vector["w"] = DataVector.w;
    JSON[Name] = {typeid(SapphireEngine::Vec4).hash_code(), Vector};
}

void SapphireEngine::Vec4::SendToLua(lua_State *L)
{
    glm::vec4& vecValue = std::any_cast<glm::vec4&>(data);
    lua_newtable(L);
    lua_pushnumber(L, vecValue.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vecValue.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, vecValue.z);
    lua_setfield(L, -2, "z");
    lua_pushnumber(L, vecValue.w);
    lua_setfield(L, -2, "w");
}

void SapphireEngine::Vec4::GetFromLua(lua_State *L)
{
    glm::vec4& vecValue = std::any_cast<glm::vec4&>(data);
    lua_getfield (L, -1, "x");
    vecValue.x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "y");
    vecValue.y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "z");
    vecValue.z = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "w");
    vecValue.z = lua_tonumber(L, -1);
    lua_pop(L, 1);
    data = vecValue;
}

void SapphireEngine::Vec4::Load(const nlohmann::json &jsonArray)
{
    glm::vec4 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    vector.z = jsonArray[1]["z"].get<float>();
    vector.w = jsonArray[1]["w"].get<float>();
    
    data = vector;
}

void SapphireEngine::Color::RenderGUI()
{
    ImGui::ColorEdit4(Name.c_str(), &(*std::any_cast<glm::vec4>(&data))[0]);
}

void SapphireEngine::Color::Save(nlohmann::json &JSON)
{
    nlohmann::json Color;
    glm::vec4& DataVector = std::any_cast<glm::vec4&>(data);
    Color["r"] = DataVector.r;
    Color["g"] = DataVector.g;
    Color["b"] = DataVector.b;
    Color["a"] = DataVector.a;
    JSON[Name] = {typeid(SapphireEngine::Color).hash_code(), Color};
}

void SapphireEngine::Color::SendToLua(lua_State *L)
{
    glm::vec4& vecValue = std::any_cast<glm::vec4&>(data);
    lua_newtable(L);
    lua_pushnumber(L, vecValue.x);
    lua_setfield(L, -2, "r");
    lua_pushnumber(L, vecValue.y);
    lua_setfield(L, -2, "g");
    lua_pushnumber(L, vecValue.z);
    lua_setfield(L, -2, "b");
    lua_pushnumber(L, vecValue.w);
    lua_setfield(L, -2, "a");
}

void SapphireEngine::Color::GetFromLua(lua_State *L)
{
    glm::vec4& vecValue = std::any_cast<glm::vec4&>(data);
    lua_getfield(L, -1, "r");
    vecValue.x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "g");
    vecValue.y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "b");
    vecValue.z = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "a");
    vecValue.z = lua_tonumber(L, -1);
    lua_pop(L, 1);
    data = vecValue;
}

void SapphireEngine::Color::Load(const nlohmann::json &jsonArray)
{
    glm::vec4 vector;
    vector.x = jsonArray[1]["r"].get<float>();
    vector.y = jsonArray[1]["g"].get<float>();
    vector.z = jsonArray[1]["b"].get<float>();
    vector.w = jsonArray[1]["a"].get<float>();
    data = vector;
}
