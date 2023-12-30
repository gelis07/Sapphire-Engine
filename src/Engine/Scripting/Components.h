#pragma once
#include <map>
#include <variant>
#include "Scripting.h"

class Component
{
    using TableVariable = std::unordered_map<std::string, SapphireEngine::Variable*>;
    public:
        Component(std::string File, std::string ArgName , unsigned int ArgId, bool LuaComp=false);
        Component(const Component& comp);
        ~Component();
        virtual void CustomRendering() {};
        void ExecuteFunction(std::string Name);
        void UpdateLuaVariables();
        bool GetLuaVariables();
        void UpdateExistingVars();
        void SetLuaComponent(lua_State* ComponentsState);
        void Render();
        SapphireEngine::Variable* Get(std::string Name);
        nlohmann::ordered_json Save();
        void Load(nlohmann::ordered_json JSON);
        bool Active = true;
        lua_State* GetState() const {return L;}
        std::string GetFile() const {return m_LuaFile;}
        std::string Name;
        TableVariable Variables;
        std::unordered_map<std::string, lua_CFunction> Functions;
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

class LuaCamera : public Component
{
    public:
        LuaCamera(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp = false)
        : Component(std::move(File), std::move(ArgName), ArgId,LuaComp), BgColor("BgColor", Variables), Zoom("Zoom", Variables){
            BgColor.Get() = glm::vec4(0,0,0,1);
            Zoom.Get() = 1.0f;
        };
        SapphireEngine::Color BgColor;
        SapphireEngine::Float Zoom;
};


