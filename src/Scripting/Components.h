#pragma once
#include <map>
#include <variant>
#include "Variables.h"

class Component
{
    public:
        Component(std::string File, std::string ArgName , unsigned int ArgId, bool LuaComp=false);
        ~Component();
        void ExecuteFunction(std::string Name);
        void UpdateLuaVariables();
        bool GetLuaVariables();
        void SetLuaComponent(lua_State* ComponentsState);
        void Render();
        SapphireEngine::Variable* Get(std::string Name);
        nlohmann::json Save();
        void Load(nlohmann::json JSON);
        bool Active = true;

        lua_State* GetState() {return L;}
        std::string GetFile() {return m_LuaFile;}
        std::string Name;
        std::unordered_map<std::string, SapphireEngine::Variable*> Variables;
        std::unordered_map<std::string, lua_CFunction> Functions;
    protected:
        virtual void test() {} //! This... is just so dumb.
        lua_State* L = nullptr;
        std::string m_LuaFile = "";
        unsigned int m_ID;
};

/*
    A component class should contain a SapphireEngine type if it should be rendered on the UI, Saved on the scene file and send/received from lua.
    Also it should be Initialized on the Component constructor with the variable name and the Variables map (should stay the same for most variables). 
*/
class Transform : public Component
{
    public:
        Transform(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(File, ArgName, ArgId, LuaComp), Position("Position", Variables), Rotation("Rotation", Variables), Size("Size", Variables)
        {
            Variables["Position"]->AnyValue() = glm::vec3(0);
            Variables["Rotation"]->AnyValue() = glm::vec3(0);
            Variables["Size"]->AnyValue() = glm::vec3(0);
        };
        SapphireEngine::Vec3 Position;
        SapphireEngine::Vec3 Rotation;
        SapphireEngine::Vec3 Size;
};

class Renderer : public Component
{
    public:
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(File, ArgName, ArgId, LuaComp), Color("Color", Variables) {
            Variables["Color"]->AnyValue() = glm::vec4(1);
        };
        std::shared_ptr<Shapes::Shape> shape;
        Shapes::Type Type;
        SapphireEngine::Color Color;
        void Render(bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom, bool Viewport);
};

class Camera : public Component
{
    public:
        Camera(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(File, ArgName, ArgId, LuaComp), BgColor("BgColor", Variables), Zoom("Zoom", Variables){
            Variables["BgColor"]->AnyValue() = glm::vec4(1);
            Variables["Zoom"]->AnyValue() = 1.0f;
        };
        SapphireEngine::Color BgColor;
        SapphireEngine::Float Zoom;
        void Render(bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom, bool Viewport);
};
