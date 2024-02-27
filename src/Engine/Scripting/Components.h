#pragma once
#include <map>
#include <variant>
#include "Scripting.h"
#include "Editor/Variables.h"

struct LuaFunction
{
    int Arugments = 0;
    int Return = 0;
    std::string Name;
};

class Component
{
    using TableVariable = std::unordered_map<std::string, SapphireEngine::Variable*>;
    public:
        inline static std::unordered_map<std::string, std::function<std::shared_ptr<Component>(Object*)>> ComponentTypeRegistry;
        template <typename T>
        static void RegisterComponentType(std::function<std::shared_ptr<T>(Object*)> func, const std::string& ClassName) {
            ComponentTypeRegistry[ClassName] = func;
        }
    public:
        Component(std::string File, std::string ArgName , unsigned int ArgId, ObjectRef obj);
        Component(const std::string& ArgName, ObjectRef obj) : Name(ArgName), Parent(obj) {} // This will be used for the non lua components.
        Component(const Component& comp);
        ~Component();
        virtual void CustomRendering() {};
        void ExecuteFunction(std::string Name);
        void UpdateLuaVariables();
        bool GetLuaVariables();
        void UpdateExistingVars();
        void SetLuaComponent(lua_State* ComponentsState);
        void Render();
        ObjectRef Parent;
        SapphireEngine::Variable* Get(std::string Name);
        nlohmann::ordered_json Save();
        void Load(nlohmann::ordered_json JSON);
        bool Active = true;
        lua_State* GetState() const {return L;}
        std::string GetFile() const {return m_LuaFile;}
        std::string Name;
        TableVariable Variables;
        void UpdateVariable(const std::string& Name, SapphireEngine::Variable* var);
        std::unordered_map<std::string, lua_CFunction> Functions;
        std::unordered_map<std::string, LuaFunction> LuaFunctions;


    protected:
        TableVariable VariablesToUpdate;
        lua_State* L = nullptr;
        int m_ID;
        std::string m_LuaFile;
};

/*
    A component class should contain a SapphireEngine type if it should be rendered on the UI, Saved on the scene file and send/received from lua.
    Also, it should be Initialized on the Component constructor with the variable name and the Variables map (should stay the same for most variables).
*/