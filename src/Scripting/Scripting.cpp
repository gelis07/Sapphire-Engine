#include "Scripting.h"


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


std::vector<LuaTableIt> ScriptingEngine::GetTable(lua_State* L, std::string Name, std::vector<std::string> SubTables) {
    lua_getglobal(L, Name.c_str());

    for (size_t i = 0; i < SubTables.size(); i++)
    {
        lua_getfield(L, -1, SubTables[i].c_str()); 
    }
    
    std::vector<LuaTableIt> TableValues;

    lua_pushnil(L);

    while (lua_next(L, -2))
    {
        lua_pushvalue(L, -2);
        LuaTableIt value;
        int i = 0;
        if(lua_isstring(L, -1)){
            value.Key = std::string(lua_tostring(L, -1));
            i++;
        }else if(lua_isinteger(L, -1)){
            value.Key = (int)lua_tointeger(L, -1);
            i++;
        }

        if(lua_isnumber(L, -2)){
            value.Contents = (float)lua_tonumber(L, -2);
            value.Type = LUA_TNUMBER;
            i++;
        }else if(lua_isboolean(L, -2)){
            value.Contents = lua_isboolean(L, -2);
            value.Type = LUA_TBOOLEAN;
            i++;
        }else if(lua_istable(L, -2)){
            std::string KeyName = std::holds_alternative<std::string>(value.Key) ? std::get<std::string>(value.Key) : std::to_string(std::get<int>(value.Key));
            SubTables.push_back(KeyName);
            value.Contents = ScriptingEngine::GetTable(L, Name, SubTables);
            SubTables.pop_back();
            value.Type = LUA_TTABLE;
            i++;
        }else if(lua_isstring(L, -2)){
            value.Contents = std::string(lua_tostring(L, -2));
            value.Type = LUA_TSTRING;
            i++;
        }
        lua_pop(L, i);
        TableValues.push_back(value);
    }
    int test = SubTables.size();
    lua_pop(L, 1 + SubTables.size());
    return TableValues;
}