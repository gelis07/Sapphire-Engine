#pragma once
#include "Utilities.hpp"
#include <functional>
#include "Objects/Objects.h"
#include "Graphics/Animation.h"

namespace LuaUtilities{

    static int log(lua_State *L);
    static int Clamp(lua_State *L);
    static int KeyPress(lua_State *L);
    static int KeyPressDown(lua_State *L);
    static int Click(lua_State *L);
    static int LoadScene(lua_State *L);
    static int GetObject(lua_State *L);
    static int DeleteObject(lua_State *L);
    static int GetCurrentScene(lua_State* L);
    static int GetCameraPos(lua_State* L);
    static int GetMouseCoord(lua_State* L);
    static int SetCameraPos(lua_State* L);
    static int GetCameraSize(lua_State* L);
    static int SetCameraSize(lua_State* L);
    static int CreateObject(lua_State* L);
    static int LoadObjectPrefab(lua_State* L);
    static int GetDeltaTime(lua_State* L);
    static int SetObject(lua_State* L);
    static int Vector(lua_State* L);
    int luaopen_SapphireEngine(lua_State* L);
}