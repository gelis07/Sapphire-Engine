#pragma once
#include "Utilities.hpp"
#include "json.hpp"
#include "Imgui/imgui_markdown.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui_stdlib.h"

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
            virtual void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) = 0;
            virtual void Save(nlohmann::json& JSON) = 0;
            virtual void Load(const nlohmann::json& jsonArray) = 0;
            virtual void SendToLua(lua_State* L) = 0;
            virtual void GetFromLua(lua_State* L) = 0;
            void ShowOnInspector(bool state) {m_ShowOnInspector = state;}
            void SaveVariable(bool state) {m_SaveVariable = state;}
            void CommunicateWithLua(bool state) {m_CommunicateWithLua = state;}
            const std::string GetName() {return Name;}
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
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
    };

    class Bool : public Variable{
        public:
            Bool(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
    };

    class String : public Variable{
        public:
            String(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            ImGuiInputTextFlags Flags = 0;
    };
    class Vec2 : public Variable{
        public:
            Vec2(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
    };
    class Vec3 : public Variable{
        public:
            Vec3(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
    };
    class Vec4 : public Variable{
        public:
            Vec4(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
    };
    class Color : public Variable{
        public:
            Color(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::json& jsonArray) override;
            ImGuiColorEditFlags Flags = 0;
    };
    class LuaTable : public Variable{
        public:
            LuaTable(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
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
