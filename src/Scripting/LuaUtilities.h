#pragma once
#include "Utilities.hpp"
#include <functional>
#include "Objects/Objects.h"
#include "Engine/Views/SceneEditor.h"
#include "Graphics/Shapes.h"

namespace LuaUtilities{

    static int log(lua_State *L);
    static int Clamp(lua_State *L);
    static int KeyPress(lua_State *L);
    static int LoadScene(lua_State *L);
    static int GetObject(lua_State *L);
    static int GetCurrentScene(lua_State* L);
    static int GetCameraPos(lua_State* L);
    static int SetCameraPos(lua_State* L);
    static int CreateObject(lua_State* L);
    static int LoadObjectPrefab(lua_State* L);
    int luaopen_SapphireEngine(lua_State* L);
}