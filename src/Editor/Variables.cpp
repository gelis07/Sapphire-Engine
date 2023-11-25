#include "Variables.h"

void SapphireEngine::Float::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables){
    if(!m_ShowOnInspector) return;
    ImGui::DragFloat(Name.c_str(), &data, SliderSpeed,Min,Max,Format,Flags);

    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Float::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    JSON[Name] = {typeid(SapphireEngine::Float).hash_code(), data};
}

void SapphireEngine::Float::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    lua_pushnumber(L, data);
}

void SapphireEngine::Float::GetFromLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    data = (float)lua_tonumber(L, -1);
}

void SapphireEngine::Float::Load(const nlohmann::json &jsonArray)
{
    if(!m_SaveVariable) return;
    data = jsonArray[1].get<float>();
}

void SapphireEngine::Bool::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::Checkbox(Name.c_str(), &data);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Bool::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    JSON[Name] = {typeid(SapphireEngine::Bool).hash_code(), data};
}

void SapphireEngine::Bool::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    lua_pushboolean(L, data);
}

void SapphireEngine::Bool::GetFromLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    data = lua_toboolean(L, -1) == true;
}

void SapphireEngine::Bool::Load(const nlohmann::json &jsonArray)
{
    if(!m_SaveVariable) return;
    data = jsonArray[1].get<bool>();
}

void SapphireEngine::String::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::InputText(Name.c_str(), &data, Flags);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::String::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    JSON[Name] = {typeid(SapphireEngine::String).hash_code(), data};
}

void SapphireEngine::String::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    lua_pushstring(L, data.c_str());
}

void SapphireEngine::String::GetFromLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    data = std::string(lua_tostring(L, -1));
}

void SapphireEngine::String::Load(const nlohmann::json &jsonArray)
{
    if(!m_SaveVariable) return;
    data = jsonArray[1].get<std::string>();
}

void SapphireEngine::Vec2::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::DragFloat2(Name.c_str(), glm::value_ptr(data), SliderSpeed, Min, Max, Format, Flags);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Vec2::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    nlohmann::json Vector;
    glm::vec2& DataVector = data;
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    JSON[Name] = {typeid(SapphireEngine::Vec2).hash_code(), Vector};
}

void SapphireEngine::Vec2::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    glm::vec2& vecValue = data;
    lua_newtable(L);
    lua_pushnumber(L, vecValue.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, vecValue.y);
    lua_setfield(L, -2, "y");
}

void SapphireEngine::Vec2::GetFromLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    glm::vec2& vecValue = data;
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
    if(!m_SaveVariable) return;
    glm::vec2 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    data = vector;
}

void SapphireEngine::Vec3::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::DragFloat3(Name.c_str(), glm::value_ptr(data), SliderSpeed, Min, Max, Format, Flags);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Vec3::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    nlohmann::json Vector;
    glm::vec3& DataVector = data;
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    Vector["z"] = DataVector.z;
    JSON[Name] = {typeid(SapphireEngine::Vec3).hash_code(), Vector};
}

void SapphireEngine::Vec3::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    glm::vec3& vecValue = data;
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
    if(!m_CommunicateWithLua) return;
    glm::vec3& vecValue = data;
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
    if(!m_SaveVariable) return;
    glm::vec3 Vector;
    Vector.x = jsonArray[1]["x"].get<float>();
    Vector.y = jsonArray[1]["y"].get<float>();
    Vector.z = jsonArray[1]["z"].get<float>();
    data = Vector;
}

void SapphireEngine::Vec4::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::DragFloat4(Name.c_str(), glm::value_ptr(data), SliderSpeed, Min, Max, Format, Flags);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Vec4::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    nlohmann::json Vector;
    glm::vec4& DataVector = data;
    Vector["x"] = DataVector.x;
    Vector["y"] = DataVector.y;
    Vector["z"] = DataVector.z;
    Vector["w"] = DataVector.w;
    JSON[Name] = {typeid(SapphireEngine::Vec4).hash_code(), Vector};
}

void SapphireEngine::Vec4::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    glm::vec4& vecValue = data;
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
    if(!m_CommunicateWithLua) return;
    glm::vec4& vecValue = data;
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
    if(!m_SaveVariable) return;
    glm::vec4 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    vector.z = jsonArray[1]["z"].get<float>();
    vector.w = jsonArray[1]["w"].get<float>();
    
    data = vector;
}

void SapphireEngine::Color::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(!m_ShowOnInspector) return;
    ImGui::ColorEdit4(Name.c_str(), glm::value_ptr(data), Flags);
    if(ImGui::IsItemEdited()){
        OnChange();
        Variables[Name] = this;
    }
}

void SapphireEngine::Color::Save(nlohmann::json &JSON)
{
    if(!m_SaveVariable) return;
    nlohmann::json Color;
    glm::vec4& DataVector = data;
    Color["r"] = DataVector.r;
    Color["g"] = DataVector.g;
    Color["b"] = DataVector.b;
    Color["a"] = DataVector.a;
    JSON[Name] = {typeid(SapphireEngine::Color).hash_code(), Color};
}

void SapphireEngine::Color::SendToLua(lua_State *L)
{
    if(!m_CommunicateWithLua) return;
    glm::vec4& vecValue = data;
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
    if(!m_CommunicateWithLua) return;
    glm::vec4& vecValue = data;
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
    if(!m_SaveVariable) return;
    glm::vec4 vector;
    vector.x = jsonArray[1]["r"].get<float>();
    vector.y = jsonArray[1]["g"].get<float>();
    vector.z = jsonArray[1]["b"].get<float>();
    vector.w = jsonArray[1]["a"].get<float>();
    data = vector;
}
void SapphireEngine::LuaTable::RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables)
{
    if(ImGui::TreeNode(Name.c_str()))
    {
        for(auto& TableVariable : data)
        {
            TableVariable.second->RenderGUI(data);
            if(ImGui::IsItemEdited()){
                OnChange();
                Variables[Name] = this;
            }
        }
        ImGui::TreePop();
    }
}

void SapphireEngine::LuaTable::Save(nlohmann::json &JSON)
{
    nlohmann::json TableJSON;
    for(auto& TableVariable : data)
    {
        TableVariable.second->Save(TableJSON);
    }
    JSON[Name] = {typeid(SapphireEngine::LuaTable).hash_code(),TableJSON};
}

void SapphireEngine::LuaTable::SendToLua(lua_State *L)
{
    lua_newtable(L);
    for(auto& TableVariable : data)
    {
        TableVariable.second->SendToLua(L);
        lua_setfield(L, -2, TableVariable.first.c_str());
    }
}

void SapphireEngine::LuaTable::GetFromLua(lua_State *L)
{
}

void SapphireEngine::LuaTable::Load(const nlohmann::json &jsonArray)
{
    std::unordered_map<std::string, SapphireEngine::Variable*> Table;
    for(auto& JSONVariable : jsonArray[1].items()){

        nlohmann::json VariableArray = JSONVariable.value();

        //At least at my knowledge of programming/c++, I couldn't find a way to remove this repetitive code.
        if(VariableArray[0] == typeid(SapphireEngine::Float).hash_code()){ 
            SapphireEngine::Float* CurrentlyEditedVariable = new SapphireEngine::Float(JSONVariable.key(), Table);
            CurrentlyEditedVariable->Load(VariableArray);
        }else if(VariableArray[0] == typeid(SapphireEngine::Bool).hash_code()){
            SapphireEngine::Bool* CurrentlyEditedVariable = new SapphireEngine::Bool(JSONVariable.key(), Table);
            CurrentlyEditedVariable->Load(VariableArray);
        }else if(VariableArray[0] == typeid(SapphireEngine::String).hash_code()){
            SapphireEngine::String* CurrentlyEditedVariable = new SapphireEngine::String(JSONVariable.key(), Table);
            CurrentlyEditedVariable->Load(VariableArray);
        }
    }
    data = Table;
}
