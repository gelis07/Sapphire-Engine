#pragma once
#include <map>
#include <variant>
#include "Variables.h"
#include "PhysicsEngine/PhysicsEngine.h"
#include "Scripting.h"
#include "Graphics/Shapes.h"

class Object;

class Component
{
    using TableVariable = std::unordered_map<std::string, SapphireEngine::Variable*>;
    public:
        Component(std::string File, std::string ArgName , unsigned int ArgId, Object* Obj,bool LuaComp=false);
        ~Component();
        void ExecuteFunction(std::string Name);
        void UpdateLuaVariables();
        bool GetLuaVariables(Object* obj);
        void SetLuaComponent(lua_State* ComponentsState);
        void Render();
        SapphireEngine::Variable* Get(std::string Name);
        nlohmann::json Save();
        void Load(nlohmann::json JSON);
        bool Active = true;
        lua_State* GetState() {return L;}
        std::string GetFile() {return m_LuaFile;}
        std::string Name;
        TableVariable Variables;
        std::unordered_map<std::string, lua_CFunction> Functions;
    protected:
        TableVariable VariablesToUpdate;
        virtual void test() {} //! This... is just so dumb.
        void Reload(Object* obj);
        lua_State* L = nullptr;
        int m_ID;
        std::string m_LuaFile;
};

/*
    A component class should contain a SapphireEngine type if it should be rendered on the UI, Saved on the scene file and send/received from lua.
    Also, it should be Initialized on the Component constructor with the variable name and the Variables map (should stay the same for most variables).
*/
class Transform : public Component
{
    public:
        Transform(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId, obj,LuaComp), Position("Position", Variables), Rotation("Rotation", Variables), Size("Size", Variables)
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
        Renderer(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId, obj, LuaComp), Color("Color", Variables) {
            Variables["Color"]->AnyValue() = glm::vec4(1);
        };
        std::shared_ptr<Shapes::Shape> shape;
        Shapes::Type Type;
        SapphireEngine::Color Color;
        void Render(std::shared_ptr<Object> obj,bool&& IsSelected,glm::vec3 CameraPos,float CameraZoom, bool Viewport);
};

class Camera : public Component
{
    public:
        Camera(std::string File, std::string ArgName, unsigned int ArgId,Object* obj, bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId,obj,LuaComp), BgColor("BgColor", Variables), Zoom("Zoom", Variables){
            Variables["BgColor"]->AnyValue() = glm::vec4(0);
            Variables["Zoom"]->AnyValue() = 1.0f;
        };
        SapphireEngine::Color BgColor;
        SapphireEngine::Float Zoom;
};


class RigidBody : public Component
{
    public:
        RigidBody(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId,obj,LuaComp), Trigger("Trigger", Variables), Gravity("Gravity", Variables), Mass("Mass", Variables), Velocity("Velocity", Variables){
            Variables["Trigger"]->AnyValue() = false;
            Variables["Gravity"]->AnyValue() = true;
            Variables["Mass"]->AnyValue() = 1.0f;
            Variables["Velocity"]->AnyValue() = glm::vec3(0);
            Variables["Velocity"]->ShowOnInspector(false);
            Variables["Velocity"]->SaveVariable(false);
            Functions["Impulse"] = Impulse;
        };
        SapphireEngine::Bool Trigger;
        SapphireEngine::Bool Gravity;
        SapphireEngine::Float Mass;
        SapphireEngine::Vec3 Velocity;
        glm::vec3 VelocityLastFrame = glm::vec3(0); // This basically means that the variable will be private and will not communicate at all with the user.
        std::vector<glm::vec3> Forces;
        void CheckForCollisions(Object* current);
        void Simulate(Object *current, const float& DeltaTime);
        static int Impulse(lua_State* L);
};