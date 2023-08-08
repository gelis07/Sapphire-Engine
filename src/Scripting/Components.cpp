#include <stdint.h>
#include <cstring>
#include "Components.h"
#include "LuaUtilities.h"
#include <typeinfo>
#include "Engine/Engine.h"

Component::Component(std::string File,std::string ArgName, unsigned int ArgId, bool luaComp) :m_LuaFile(File), Name(ArgName), m_ID(ArgId)
{
    if(!luaComp) return;
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "SapphireEngine", LuaUtilities::luaopen_SapphireEngine, 0);

    //Setting up the component to access from lua.
    auto ComponentIndex = [](lua_State* L) -> int
    {
        // Get the component table from the Lua stack
        luaL_checktype(L, 1, LUA_TTABLE);

        // Check if the component table has the '__userdata' field (the component pointer)
        lua_getfield(L, 1, "__userdata");
        Component* comp = static_cast<Component*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        const char* index = luaL_checkstring(L, 2);
        if(comp->Variables.find(index) != comp->Variables.end()){
            comp->Variables.at(index)->SendToLua(L);
            return 1;
        }
        return 0;
    };
    auto ComponentNewIndex = [](lua_State* L) -> int
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
        }
        return 0;
    };

    luaL_newmetatable(L, "Component");
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, ComponentIndex);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, ComponentNewIndex);
    lua_settable(L, -3);

}

Component::~Component()
{
    if(L == nullptr) return; // This means the component is a native c++ component so the variables are stored on the stack.

    for (auto &&Variable : Variables)
    {
        delete Variable.second;
    }
    Variables.clear();
}

void Component::ExecuteFunction(std::string Name)
{
    std::string FullPath = m_LuaFile;
    UpdateLuaVariables();
    ScriptingEngine::LuaFunction(L, Name);
}
//! This could be organized a bit more
void Component::UpdateLuaVariables()
{
    if(Variables.size() == 0) return;
    for(auto const& Var : Variables)
    {
        Var.second->SendToLua(L);
        lua_setglobal(L, Var.first.c_str());
    }
}
bool isKnownModule(lua_State* L, std::string name) {
    // These are all the tables included by the lua_openlibs() function and yes that was the best way i found
    return name != "_G" && name != "package" && name != "package.loaded" && name != "package.preload" && name != "coroutine" && name != "string" && name != "table" && name != "math" && name != "io" && name != "os" && name != "debug" && name != "utf8";
}

struct LuaVariable{
    std::string Name;
    SapphireEngine::Variable* Value;
};

bool Component::GetLuaVariables()
{
    if (luaL_loadfile(L, m_LuaFile.c_str()) || lua_pcall(L, 0, 0, 0)) {
        std::stringstream ss;
        ss<< "Error loading script: " << lua_tostring(L, -1) << std::endl;
        SapphireEngine::Log(ss.str(), SapphireEngine::Error);
        lua_pop(L, 1);
        return false;
    }
    lua_getglobal(L, "_G"); // All global variables inside the lua state
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        LuaVariable var;
        var.Name = std::string(lua_tostring(L, -2));
        lua_getglobal(L, var.Name.c_str());
        int type = lua_type(L, -2);
        const char* typeName = lua_typename(L, type);
        lua_pop(L, 1);
        if (var.Name.c_str() && !lua_isfunction(L, -1)) {
            //! Got to implement tables!
            // if(type == LUA_TTABLE && isKnownModule(L, var.first.c_str())){
            //     std::vector<LuaTableIt> test = ScriptingEngine::GetTable(L, std::string(var.first), {});
            //     var.second.Contents = test;
            //     Variables[var.first] = var.second;
            //     lua_pop(L, 1);
            //     continue;
            // }
            const char* VarValue = lua_tostring(L, -1); // Here this variable helps me to decide whether the variable is from the user and not from lua's packages
            //Checking up here because lua_tostring(L, -1) returns 0x0 for false and the if statement returns false and doesn't register the variable
            if(type == LUA_TBOOLEAN){
                var.Value = new SapphireEngine::Bool(var.Name, Variables);
                var.Value->AnyValue() = !(VarValue == nullptr);
                Variables[var.Name] = var.Value;
                lua_pop(L, 1);
                continue;
            }

            //Also checking for if the name is == to "_VERSION" because the _G table also contains the lua version and its not necessary to be displayed.
            if (VarValue && var.Name != "_VERSION") {
                if(type == LUA_TSTRING){
                    var.Value = new SapphireEngine::String(var.Name, Variables);
                    var.Value->AnyValue() = std::string(lua_tostring(L, -1));
                }else if(type == LUA_TNUMBER){
                    var.Value = new SapphireEngine::Float(var.Name, Variables);
                    var.Value->AnyValue() = (float)lua_tonumber(L, -1);
                }
                Variables[var.Name] = var.Value;
                lua_pop(L, 1);
            }else{
                lua_pop(L, 1);
                continue;
            }
            
        }else{
            lua_pop(L, 1);
            continue;
        }

    }
    return true;
}

void Component::SetLuaComponent(lua_State* ComponentsState)
{
    if(L != nullptr) return;
    
    lua_newtable(ComponentsState);
    int componentTableIdx = lua_gettop(ComponentsState);

    lua_pushlightuserdata(ComponentsState, this);
    lua_setfield(ComponentsState, componentTableIdx, "__userdata");

    luaL_getmetatable(ComponentsState, "Component");
    lua_setmetatable(ComponentsState, componentTableIdx);

}
nlohmann::json SaveTable(std::vector<LuaTableIt> &Var){
    nlohmann::json Table;
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

nlohmann::json Component::Save(){
    nlohmann::json JSONVariables;
    for(auto& Var : Variables)
    {
        Var.second->Save(JSONVariables);
    }
    return JSONVariables;
}
void Component::Render()
{
    // ImGui::Separator();
    ImGui::Text(Name.c_str());
    for (auto& Var : Variables)
    {
        Var.second->RenderGUI();
    }
}
SapphireEngine::Variable *Component::Get(std::string Name)
{
    if(Variables.find(Name) != Variables.end()){
        return Variables[Name];
    }
    SapphireEngine::Log("Couldn't find component with name: " + Name, SapphireEngine::Error);
    return nullptr;
}
void Component::Load(nlohmann::json JSON)
{
    for(auto& JSONVariable : JSON.items()){
        nlohmann::json& JsonArray = JSONVariable.value();

        SapphireEngine::Variable* CurrentlyEditedVariable = Variables[JSONVariable.key()];

        //At least at my knowledge of programming/c++, I couldn't find a way to remove this repetetive code.
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
        }
    }
}

void Renderer::Render(bool&& IsSelected ,glm::vec3 CameraPos,float CameraZoom, bool IsViewport)
{
    //That means that the object is an empty
    if(shape == nullptr) return;
    // Here it renders the object's outline to indicate that the current object is selected
    if(IsSelected){
        shape->Render(CameraPos ,CameraZoom,true, shape->Wireframe(), IsViewport);
    }
    shape->Render(CameraPos ,CameraZoom,false, shape->Wireframe(), IsViewport);
}

void RigidBody::CheckForCollisions(Object *current) {
    if(current->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
        for (auto&& object: Engine::Get().GetActiveScene()->Objects) {
            if(object->Name == "MainCamera" || object.get() == current) continue;
            if(object->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
                PhysicsEngine::RectanglexRectangle(object, current);
            }else{
                PhysicsEngine::CirclexRectangle(object, current);
            }
        }
    }else{
        for (auto&& object: Engine::Get().GetActiveScene()->Objects) {
            if(object->Name == "MainCamera" || object.get() == current) continue;
            if(object->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
                PhysicsEngine::CirclexRectangle(object, current);
            }else{
                PhysicsEngine::CirclexCircle(object, current);
            }
        }
    }
}

void RigidBody::Simulate() {

}
