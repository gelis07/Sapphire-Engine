#pragma once
#include <map>
#include <variant>
#include "Scripting.h"
#include "json.hpp"
#include "UI/Windows.h"
#define GetVariable(component, name, type) \
    std::get<type>(component->Variables[name].Contents)

    

struct Variable{
    int Type;
    std::variant<float, bool, std::string, std::vector<LuaTableIt>, glm::vec2, glm::vec3, glm::vec4> Contents; // The value of the lua variable
    //A vector of Lua Table Iterators (Thats what It stands for) is a lua table
};


class Component
{
    public:
        Component(std::string File, std::string ArgName , unsigned int ArgId, bool LuaComp=false);
        void ExecuteFunction(std::string Name);
        void UpdateLuaVariables();
        bool GetLuaVariables();
        void SetLuaComponent(lua_State* ComponentsState);
        void Render();
        nlohmann::json Save();
        void Load(nlohmann::json JSON);
        bool Active = true;

        lua_State* GetState() {return L;}
        std::string GetFile() {return LuaFile;}
        std::string Name;
        std::unordered_map<std::string, Variable> Variables;
        std::unordered_map<std::string, lua_CFunction> Functions;
    protected:
        virtual void test() {}
        lua_State* L = nullptr;
        std::string LuaFile = "";
        unsigned int id;

};

template <typename T>
inline void AssignValue(Variable& NewVar, const nlohmann::json& jsonArray){
    SapphireEngine::Log(std::string(typeid(T).name()) + " cannot be loaded from the selected scene.", SapphireEngine::Error);
}
template <>
inline void AssignValue<float>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();
    NewVar.Contents = jsonArray[1].get<float>();
}
template <>
inline void AssignValue<std::string>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();
    NewVar.Contents = jsonArray[1].get<std::string>();
}
template <>
inline void AssignValue<bool>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();
    NewVar.Contents = jsonArray[1].get<bool>();
}
template <>
inline void AssignValue<glm::vec2>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();
    glm::vec2 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    NewVar.Contents = vector;
}
template <>
inline void AssignValue<glm::vec3>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();
    glm::vec3 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    vector.z = jsonArray[1]["z"].get<float>();
    
    NewVar.Contents = vector;
}
template <>
inline void AssignValue<glm::vec4>(Variable& NewVar, const nlohmann::json& jsonArray) {
    NewVar.Type = jsonArray[0].get<int>();

    glm::vec4 vector;
    vector.x = jsonArray[1]["x"].get<float>();
    vector.y = jsonArray[1]["y"].get<float>();
    vector.z = jsonArray[1]["z"].get<float>();
    vector.w = jsonArray[1]["w"].get<float>();
    
    NewVar.Contents = vector;
}


class Transform : public Component
{
    public:
        Transform(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(File, ArgName, ArgId, LuaComp) {
            Variables["Position"].Contents = glm::vec3(0);
            Variables["Rotation"].Contents = glm::vec3(0);
            Variables["Size"].Contents = glm::vec2(0);
        };
        glm::vec3 Position;
        glm::vec3 Rotation;
        glm::vec3 Size;
};

class Renderer : public Component
{
    public:
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(File, ArgName, ArgId, LuaComp) {
            Variables["Color"].Contents = glm::vec4(0);
        };
        std::shared_ptr<Shapes::Shape> shape;
        Shapes::Type Type;
        glm::vec4 Color = glm::vec4(0);
        void Render(bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom, bool Viewport);
};
