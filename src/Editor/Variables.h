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


class Object;
class ObjectRef{
    public:
        ObjectRef(int id) : ID(id) {}
        Object* operator->();
        bool operator==(int other);
        bool operator!=(int other);
        Object* Get();
    private:
        int ID;
};

namespace SapphireEngine{
    class Variable{
        public:
            Variable(std::string name, std::unordered_map<std::string, Variable*>& map): Name(name)
            {
                map[name] = this;
            }
            virtual void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) = 0;
            virtual void Save(nlohmann::ordered_json& JSON) = 0;
            virtual void Load(const nlohmann::ordered_json& jsonArray) = 0;
            virtual void SendToLua(lua_State* L) = 0;
            virtual void GetFromLua(lua_State* L) = 0;
            void ShowOnInspector(bool state) {m_ShowOnInspector = state;}
            void SaveVariable(bool state) {m_SaveVariable = state;}
            void CommunicateWithLua(bool state) {m_CommunicateWithLua = state;}
            const std::string GetName() {return Name;}
            void SetOnChangeFunc(const std::function<void()>& func) {OnChange = func;}
            int luaType = -1;
        protected:
            std::function<void()> OnChange = []() { };
            bool m_ShowOnInspector = true;
            bool m_SaveVariable = true;
            bool m_CommunicateWithLua = true;
            std::string Name;
    };

    class Float : public Variable{

        public:
            Float(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
            float& Get() {return data;}
            const float& Get() const {return data;}
        private:
            float data;
    };

    class Bool : public Variable{
        public:
            Bool(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            bool& Get() {return data;}
            const bool& Get() const {return data;}
        private:
            bool data;
    };

    class String : public Variable{
        public:
            String(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            ImGuiInputTextFlags Flags = 0;
            std::string& Get() {return data;}
            const std::string& Get() const {return data;}
        private:
            std::string data;
    };
    class Vec2 : public Variable{
        public:
            Vec2(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
            glm::vec2& Get() {return data;}
            const glm::vec2& Get() const {return data;}
        private:
            glm::vec2 data;
    };
    class Vec3 : public Variable{
        public:
            Vec3(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
            glm::vec3& Get() {return data;}
            const glm::vec3& Get() const {return data;}
        private:
            glm::vec3 data;
    };
    class Vec4 : public Variable{
        public:
            Vec4(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            float SliderSpeed = 1.0f;
            float Min = -INFINITY; // For no min or max just leave them on infinity.
            float Max = INFINITY;
            const char* Format = "%.3f"; // Thats the default ImGui value.
            ImGuiSliderFlags Flags = 0;
            glm::vec4& Get() {return data;}
            const glm::vec4& Get() const {return data;}
        private:
            glm::vec4 data;
    };
    class Color : public Variable{
        public:
            Color(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            ImGuiColorEditFlags Flags = 0;
            glm::vec4& Get() {return data;}
            const glm::vec4& Get() const {return data;}
        private:
            glm::vec4 data;
    };
    class LuaTable : public Variable{
        public:
            LuaTable(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map) {}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            std::unordered_map<std::string, Variable*>& Get() {return data;}
            const std::unordered_map<std::string, Variable*>& Get() const {return data;}
        private:
            std::unordered_map<std::string, Variable*> data;
    };
    class ObjectVar : public Variable{
        public:
            ObjectVar(std::string name, std::unordered_map<std::string, Variable*>& map)
            : Variable(name, map), data(-1){}
            void RenderGUI(std::unordered_map<std::string, SapphireEngine::Variable*>& Variables) override;
            void Save(nlohmann::ordered_json& JSON) override;
            void SendToLua(lua_State* L) override;
            void GetFromLua(lua_State* L) override;
            void Load(const nlohmann::ordered_json& jsonArray) override;
            ObjectRef& Get() {return data;}
            const ObjectRef& Get() const {return data;}
        private:
            ObjectRef data;
    };
}
