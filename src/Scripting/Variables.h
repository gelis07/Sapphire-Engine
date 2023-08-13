#pragma once
#include "Utilities.hpp"
#include "json.hpp"
#include "UI/Windows.h"
extern "C"
{
    #include "Lua/lua.h"
    #include "Lua/lauxlib.h"
    #include "Lua/lualib.h"
}
namespace SapphireEngine{
    class Variable{
        public:
            Variable(std::string name, std::unordered_map<std::string, Variable*>& map): Name(name)
            {
                map[name] = this;
            }
            virtual void RenderGUI() = 0;
            virtual void Save(nlohmann::json& JSON) = 0;
            virtual void Load(const nlohmann::json& jsonArray) = 0;
            virtual void SendToLua(lua_State* L) = 0;
            virtual void GetFromLua(lua_State* L) = 0;
            void ShowOnInspector(bool state) {m_ShowOnInspector = state;}
            void SaveVariable(bool state) {m_SaveVariable = state;}
            void CommunicateWithLua(bool state) {m_CommunicateWithLua = state;}
            std::any& AnyValue() {return data;} // this one just returns the std::any
            //This template returns the value with the variable defined
            template <typename T>
            T& value();
        protected:
            bool m_ShowOnInspector = true;
            bool m_SaveVariable = true;
            bool m_CommunicateWithLua = true;
            std::string Name;
            std::any data;
    };

    class Float : public Variable{

        public:
            Float(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };

    class Bool : public Variable{
        public:
            Bool(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };

    class String : public Variable{
        public:
            String(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    class Vec2 : public Variable{
        public:
            Vec2(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    class Vec3 : public Variable{
        public:
            Vec3(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    class Vec4 : public Variable{
        public:
            Vec4(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    class Color : public Variable{
        public:
            Color(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    class LuaTable : public Variable{
        public:
            LuaTable(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI() override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };
    template <typename T>
    inline T& Variable::value()
    {
        T& test = std::any_cast<T&>(data);
        return test;
    }
}
