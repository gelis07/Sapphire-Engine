#pragma once
#include "Utilities.hpp"
#include "Variables.h"

struct LuaTableIt{
    std::variant<int, std::string> Key;
    int Type;
    std::variant<float, bool, std::string, std::vector<LuaTableIt>> Contents; // The value of the lua table
};

extern "C"
{
    #include "Lua/lua.h"
    #include "Lua/lauxlib.h"
    #include "Lua/lualib.h"
}


class ScriptingEngine
{
    public:
        static bool CheckLua(lua_State *L, int r);
        static std::unordered_map<std::string, SapphireEngine::Variable*> GetTable(lua_State *L, std::string Name, std::vector<std::string> SubTables);
        static void LuaFunction(lua_State *L,std::string Name);
};