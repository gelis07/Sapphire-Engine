#include <stdint.h>
#include <cstring>
#include "Components.h"
#include "LuaUtilities.h"
#include "Editor/UI/FileExplorer/FileExplorer.h"
#include <typeinfo>
#include "Engine/Engine.h"

std::string FuncName = "";
static int CallLuaFunc(lua_State* L){
    int n = lua_gettop(L);
    // Get the component table from the Lua stack
    luaL_checktype(L, 1, LUA_TTABLE);
    // Check if the component table has the '__userdata' field (the component pointer)
    lua_getfield(L, 1, "__userdata");
    Component* comp = static_cast<Component*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    lua_getglobal(comp->GetState(), FuncName.c_str());
    for (int i = 2; i <= n; ++i) {
        // Get the type of the argument
        int type = lua_type(L, i);

        // Depending on the type, push the argument back onto the stack
        switch (type) {
            case LUA_TNIL:
                lua_pushnil(comp->GetState());
                break;
            case LUA_TBOOLEAN:
                lua_pushboolean(comp->GetState(), lua_toboolean(L, i));
                break;
            case LUA_TNUMBER:
                lua_pushnumber(comp->GetState(), luaL_checknumber(L, i));
                break;
            case LUA_TSTRING:
                lua_pushstring(comp->GetState(), luaL_checkstring(L, i));
                break;
            default:
                lua_pushnil(comp->GetState());
                break;
        }
    }
    lua_pcall(comp->GetState(), n-1, 1, 0);
    return 1;
}
static int ComponentIndex(lua_State* L)
{
    int n = lua_gettop(L);
    // Get the component table from the Lua stack
    luaL_checktype(L, 1, LUA_TTABLE);
    // Check if the component table has the '__userdata' field (the component pointer)
    lua_getfield(L, 1, "__userdata");
    Component* comp = static_cast<Component*>(lua_touserdata(L, -1));
    const char* index = luaL_checkstring(L, 2);
    lua_pop(L, 1);

    if(comp->LuaFunctions.find(index) != comp->LuaFunctions.end()){
        FuncName = std::string(index); //Store the name so I can get the global on the other function.
        //The reason for pushing the function and calling it right away is to get the arguemts that may be passed into the function.
        lua_pushcfunction(L, CallLuaFunc);
        return 1;
    }
    if(comp->Variables.find(index) != comp->Variables.end()){
        comp->Variables.at(index)->SendToLua(L);
        return 1;
    }
    return 0;
};
static int ComponentNewIndex(lua_State* L)
{
    // Get the component table from the Lua stack
    luaL_checktype(L, 1, LUA_TTABLE);

    // Check if the component table has the '__userdata' field (the component pointer)
    lua_getfield(L, 1, "__userdata");
    Component* comp = static_cast<Component*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    const char* index = luaL_checkstring(L, 2);

    if(comp->Variables.find(index) != comp->Variables.end()){
        comp->Variables.at(index)->GetFromLua(L);
        comp->UpdateVariable(comp->Variables.at(index)->GetName(), comp->Variables.at(index));
    }
    return 0;
};




Component::Component(std::string File,std::string ArgName, unsigned int ArgId) :m_LuaFile(File), Name(ArgName), m_ID(ArgId)
{
    // lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "SapphireEngine", LuaUtilities::luaopen_SapphireEngine, 0);

    luaL_newmetatable(L, "Component");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ComponentIndex);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, ComponentNewIndex);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);
}


Component::Component(const Component &comp)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "SapphireEngine", LuaUtilities::luaopen_SapphireEngine, 0);

    luaL_newmetatable(L, "ObjectMetaTable");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);


    luaL_newmetatable(L, "Component");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ComponentIndex);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, ComponentNewIndex);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);
    Name = comp.Name;
    m_LuaFile = comp.GetFile();
}

Component::~Component()
{
    if(L == nullptr) return; // This means the component is a native c++ component so the variables are stored on the stack.

    for (auto &&Variable : Variables)
    {
        delete(Variable.second);
    }
    Variables.clear();
    lua_close(L);
}

void Component::ExecuteFunction(std::string Name)
{
    UpdateLuaVariables();
    ScriptingEngine::LuaFunction(L, Name);
}
void Component::UpdateLuaVariables()
{
    for(auto const& Var : VariablesToUpdate)
    {
        Var.second->SendToLua(L);
        if(typeid(Var.second).hash_code() != typeid(SapphireEngine::LuaTable).hash_code())
            lua_setglobal(L, Var.first.c_str());
        else{
            lua_getglobal(L, Var.second->GetName().c_str());
            for(auto&& x : ((SapphireEngine::LuaTable*)(Var.second))->Get()){
                lua_setfield(L, -1, x.first.c_str());
            }
        }
    }
    VariablesToUpdate.clear();
}
const std::vector<std::string> luaLibraries = {
    "next", "assert", "table", "rawlen", "rawequal", "string", "load", "utf8",
    "setmetatable", "ipairs", "tostring", "warn", "debug", "collectgarbage",
    "error", "package", "_VERSION", "getmetatable", "select", "require",
    "tonumber", "math", "xpcall", "loadfile", "print", "io", "os", "rawset",
    "dofile", "rawget", "pcall", "_G", "pairs", "type", "coroutine"
};
bool isKnownModule(lua_State* L, std::string name) {
    // These are all the tables included by the lua_openlibs() function and yes that was the best way i found
    return !(std::find(luaLibraries.begin(), luaLibraries.end(), name) == luaLibraries.end());
}

struct LuaVariable{
    std::string Name;
    SapphireEngine::Variable* Value;
};

bool Component::GetLuaVariables()
{
    if(L == nullptr) return true;
    lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "SapphireEngine", LuaUtilities::luaopen_SapphireEngine, 0);

    luaL_newmetatable(L, "ObjectMetaTable");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);


    luaL_newmetatable(L, "Component");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ComponentIndex);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, ComponentNewIndex);
    lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);



    if (!ScriptingEngine::CheckLua(L, luaL_dofile(L, (Engine::GetMainPath() + GetFile()).c_str())))
    {
        std::stringstream ss;
        ss << "Error loading script: " << lua_tostring(L, -1) << std::endl;
        Log(ss.str(), SapphireEngine::Error);
        lua_pop(L, 1);
    }

    lua_getglobal(L, "_G"); // All global variables inside the lua state
    lua_pushnil(L);
    std::vector<std::string> VarsTested = {};
    while (lua_next(L, -2) != 0)
    {
        //Check whether its a user defined variable or not.
        if(isKnownModule(L, std::string(lua_tostring(L, -2)))) {
            lua_pop(L ,1);
            continue;
        }

        LuaVariable var;
        var.Name = std::string(lua_tostring(L, -2));
        lua_getglobal(L, var.Name.c_str());
        int type = lua_type(L, -2);
        const char* typeName = lua_typename(L, type);
        lua_pop(L, 1);

        //If its a function we dont want it.
        if(lua_isfunction(L, -1)) {
            if(var.Name == "OnCollision" || var.Name == "OnUpdate" || var.Name == "OnStart"){
                lua_pop(L ,1);
                continue;
            }
            lua_Debug ar;
            if (lua_getinfo(L, ">u", &ar) == 0) {
                std::cerr << "Error getting function information for " << var.Name << std::endl;
                lua_pop(L, 1);
                return false;
            }
            LuaFunctions[var.Name] = {ar.nparams, (ar.isvararg == 0), var.Name};
            continue;
        }

        VarsTested.push_back(var.Name);

        if(Variables.find(var.Name) != Variables.end() && Variables.find(var.Name)->second->luaType == type){
            lua_pop(L ,1);
            continue;
        }
        if (type == LUA_TUSERDATA) {
            if(ObjectRef* obj = (ObjectRef*)lua_touserdata(L, -1)){
                var.Value = new SapphireEngine::ObjectVar(var.Name, Variables);
                ((SapphireEngine::ObjectVar*)var.Value)->Get() = *obj;
            }
        }
        else if(type == LUA_TTABLE){
            lua_getfield(L, -1, "x");
            if(!lua_isnil(L, -1)){
                float x = lua_tonumber(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, -1, "y");
                if(!lua_isnil(L, -1)){
                    float y = lua_tonumber(L, -1);
                    lua_pop(L, 1);
                    var.Value = new SapphireEngine::Vec2(var.Name, Variables);
                    ((SapphireEngine::Vec2*)var.Value)->Get() = glm::vec2(x,y);
                }else{
                    var.Value = new SapphireEngine::LuaTable(var.Name, Variables);
                    std::unordered_map<std::string, SapphireEngine::Variable*> test = ScriptingEngine::GetTable(L, std::string(var.Name), {});
                    ((SapphireEngine::LuaTable*)var.Value)->Get() = test;
                }
            }else{
                var.Value = new SapphireEngine::LuaTable(var.Name, Variables);
                std::unordered_map<std::string, SapphireEngine::Variable*> test = ScriptingEngine::GetTable(L, std::string(var.Name), {});
                ((SapphireEngine::LuaTable*)var.Value)->Get() = test;
            }
        }
        else if(type == LUA_TBOOLEAN){
            var.Value = new SapphireEngine::Bool(var.Name, Variables);
            ((SapphireEngine::Bool*)var.Value)->Get() = lua_toboolean(L, -1);
        }
        else if(type == LUA_TSTRING){
            var.Value = new SapphireEngine::String(var.Name, Variables);
            ((SapphireEngine::String*)var.Value)->Get() = std::string(lua_tostring(L, -1));
        }else if(type == LUA_TNUMBER){
            var.Value = new SapphireEngine::Float(var.Name, Variables);
            ((SapphireEngine::Float*)var.Value)->Get() = (float)lua_tonumber(L, -1);
        }
        var.Value->luaType = type;
        lua_pop(L, 1);
    }
    TableVariable NewVars = Variables;
    for (auto &&vars : VarsTested)
    {
        if(NewVars.find(vars) != NewVars.end()){
            NewVars.erase(vars);
        }
    }
    for (auto &&vars : NewVars)
    {
        Variables.erase(vars.first);
    }
    
    return true;
}

void Component::UpdateExistingVars()
{
    for (auto const& variable : Variables)
    {
        lua_getglobal(L, variable.first.c_str());
        variable.second->GetFromLua(L);
    }
}

void Component::SetLuaComponent(lua_State* ComponentsState)
{
    luaL_newmetatable(ComponentsState, "Component");
    lua_pushstring(ComponentsState, "__index");
    lua_pushcfunction(ComponentsState, ComponentIndex);
    lua_settable(ComponentsState, -3);
    lua_pushstring(ComponentsState, "__newindex");
    lua_pushcfunction(ComponentsState, ComponentNewIndex);
    lua_settable(ComponentsState, -3);

    lua_newtable(ComponentsState);
    int componentTableIdx = lua_gettop(ComponentsState);

    lua_pushlightuserdata(ComponentsState, this);
    lua_setfield(ComponentsState, componentTableIdx, "__userdata");

    for(auto&& function : this->Functions){
        lua_pushcfunction(ComponentsState, function.second);
        lua_setfield(ComponentsState, componentTableIdx, function.first.c_str());
    }

    luaL_getmetatable(ComponentsState, "Component");
    lua_setmetatable(ComponentsState, componentTableIdx);

}
nlohmann::ordered_json SaveTable(std::vector<LuaTableIt> &Var){
    nlohmann::ordered_json Table;
    for(auto& TableVariable : Var){
        std::string VarName;
        if(std::holds_alternative<int>(TableVariable.Key)){
            VarName = std::to_string(std::get<int>(TableVariable.Key));
        }else if(std::holds_alternative<std::string>(TableVariable.Key)){
            VarName = std::get<std::string>(TableVariable.Key);
        }
        if(TableVariable.Type == LUA_TNUMBER){
            Table[VarName] = {TableVariable.Type, std::get<float>(TableVariable.Contents)};
        }
        else if(TableVariable.Type == LUA_TSTRING){
            Table[VarName] = {TableVariable.Type, std::get<std::string>(TableVariable.Contents)};
        }
        else if(TableVariable.Type == LUA_TBOOLEAN){
            Table[VarName] = {TableVariable.Type, std::get<bool>(TableVariable.Contents)};
        }else if(TableVariable.Type == LUA_TTABLE){
            Table[VarName] = {LUA_TTABLE, SaveTable(std::get<std::vector<LuaTableIt>>(TableVariable.Contents))};
        }
    }
    return Table;
}

nlohmann::ordered_json Component::Save(){
    nlohmann::ordered_json JSONVariables;
    for(auto& Var : Variables)
    {
        Var.second->Save(JSONVariables);
    }
    return JSONVariables;
}
void Component::Render()
{
    ImGui::Text(Name.c_str());
    for (auto& Var : Variables)
    {
        Var.second->RenderGUI(VariablesToUpdate);
    }
    HierachyDrop.CalcDragging();
    CustomRendering();
}

SapphireEngine::Variable *Component::Get(std::string Name)
{
    if(Variables.find(Name) != Variables.end()){
        return Variables[Name];
    }
    SapphireEngine::Log("Couldn't find component with name: " + Name, SapphireEngine::Error);
    return nullptr;
}
void Component::Load(nlohmann::ordered_json JSON)
{
    if(L != nullptr){
        GetLuaVariables();
    }
    for(auto& JSONVariable : JSON.items()){
        if(L != nullptr && Variables.find(JSONVariable.key()) == Variables.end()) continue;
        nlohmann::ordered_json& JsonArray = JSONVariable.value();

        SapphireEngine::Variable* CurrentlyEditedVariable = Variables[JSONVariable.key()];

        if(JsonArray[0] == typeid(SapphireEngine::Float).hash_code()){ 
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Float(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::Bool).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Bool(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::String).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::String(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::Vec2).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Vec2(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::Vec3).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Vec3(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::Vec4).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Vec4(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::Color).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::Color(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::LuaTable).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::LuaTable(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }else if(JsonArray[0] == typeid(SapphireEngine::ObjectVar).hash_code()){
            if(CurrentlyEditedVariable == nullptr)
                CurrentlyEditedVariable = new SapphireEngine::ObjectVar(JSONVariable.key(), Variables);
            CurrentlyEditedVariable->Load(JsonArray);
        }
        VariablesToUpdate[CurrentlyEditedVariable->GetName()] = CurrentlyEditedVariable;
    }
}

void Component::UpdateVariable(const std::string &Name, SapphireEngine::Variable *var)
{
    VariablesToUpdate[Name] = var;
}
