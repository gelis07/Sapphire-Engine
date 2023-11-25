#include "Scripting.h"
#include "Engine/Engine.h"

bool ScriptingEngine::CheckLua(lua_State *L, int r){
    if(r != LUA_OK)
    {
        std::string errormsg = lua_tostring(L, -1);
        std::cout << errormsg << std::endl;
        Log(std::move(errormsg), SapphireEngine::Error);
        return false;
    }
    return true;
}


void ScriptingEngine::LuaFunction(lua_State *L,std::string Name){
    lua_getglobal(L, Name.c_str());
    if(lua_isfunction(L, -1)){
        CheckLua(L, lua_pcall(L, 0, 1, 0));
    }
}


std::unordered_map<std::string, SapphireEngine::Variable*> ScriptingEngine::GetTable(lua_State* L, std::string Name, std::vector<std::string> SubTables) {
    lua_getglobal(L, Name.c_str());

    for (size_t i = 0; i < SubTables.size(); i++)
    {
        lua_getfield(L, -1, SubTables[i].c_str()); 
    }
    
    std::unordered_map<std::string, SapphireEngine::Variable*> TableValues;

    lua_pushnil(L);

    while (lua_next(L, -2))
    {
        lua_pushvalue(L, -2);
        SapphireEngine::Variable* Value;
        int i = 0;
        std::string Key;
        if(lua_isstring(L, -1)){
            Key = std::string(lua_tostring(L, -1));
            i++;
        }else if(lua_isinteger(L, -1)){
            Key = std::to_string(lua_tointeger(L, -1));
            i++;
        }

        if(lua_isnumber(L, -2)){
            Value = new SapphireEngine::Float(Key, TableValues);
            ((SapphireEngine::Float*)Value)->Get() = (float)lua_tonumber(L, -2);
            i++;
        }else if(lua_isboolean(L, -2)){
            Value = new SapphireEngine::Bool(Key, TableValues);
            ((SapphireEngine::Bool*)Value)->Get() = lua_isboolean(L, -2);
            i++;
        }else if(lua_istable(L, -2)){
            SubTables.push_back(Key);
            Value = new SapphireEngine::LuaTable(Key, TableValues);
            ((SapphireEngine::LuaTable*)Value)->Get() = ScriptingEngine::GetTable(L, Name, SubTables);
            SubTables.pop_back();
            i++;
        }else if(lua_isstring(L, -2)){
            Value = new SapphireEngine::String(Key, TableValues);
            ((SapphireEngine::String*)Value)->Get() = std::string(lua_tostring(L, -1));
            i++;
        }
        lua_pop(L, i);
    }
    lua_pop(L, 1 + SubTables.size());
    return TableValues;
}