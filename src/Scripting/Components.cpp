#include "Objects/Objects.h"
#include <stdint.h>
#include <cstring>
#include "Components.h"
#include "LuaUtilities.h"
#include <typeinfo>
#define VariantHolds(Type, variant, ...) \
    if(std::holds_alternative<Type>(variant)){ \
        __VA_ARGS__; \
    }

void GET_VAR_FROM_LUA(lua_State* L, Variable var) {
    if(std::holds_alternative<float>(var.Contents)){
        lua_pushnumber(L, std::get<float>(var.Contents));
    } else if(std::holds_alternative<bool>(var.Contents)) {
        lua_pushboolean(L, std::get<bool>(var.Contents));
    }
    else if(std::holds_alternative<std::string>(var.Contents)) {
        lua_pushstring(L, std::get<std::string>(var.Contents).c_str());
    }
    else if(std::holds_alternative<glm::vec2>(var.Contents)) {
        glm::vec2 vecValue = std::get<glm::vec2>(var.Contents);
        lua_newtable(L);
        lua_pushnumber(L, vecValue.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, vecValue.y);
        lua_setfield(L, -2, "y");
    }
    else if(std::holds_alternative<glm::vec3>(var.Contents)) {
        glm::vec3 vecValue = std::get<glm::vec3>(var.Contents);
        lua_newtable(L);
        lua_pushnumber(L, vecValue.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, vecValue.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, vecValue.z);
        lua_setfield(L, -2, "z");
    }
    else if(std::holds_alternative<glm::vec4>(var.Contents)) {
        glm::vec4 vecValue = std::get<glm::vec4>(var.Contents);
        lua_newtable(L);
        lua_pushnumber(L, vecValue.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, vecValue.y);
        lua_setfield(L, -2, "y");
        lua_pushnumber(L, vecValue.z);
        lua_setfield(L, -2, "z");
        lua_pushnumber(L, vecValue.w);
        lua_setfield(L, -2, "w");
    }
}
void SET_VAR_FROM_LUA(lua_State* L, Component* comp,Variable var, std::string name) {
    if(std::holds_alternative<float>(var.Contents)){
        GetVariable(comp, name, float) = lua_tonumber(L, -1);
    } else if(std::holds_alternative<bool>(var.Contents)) {
        GetVariable(comp, name, bool) = lua_toboolean(L, -1);
    }
    else if(std::holds_alternative<std::string>(var.Contents)) {
        GetVariable(comp, name, std::string) = std::string(lua_tostring(L, -1));
    }
    else if(std::holds_alternative<glm::vec2>(var.Contents)) {
        glm::vec2& vecValue = std::get<glm::vec2>(var.Contents);
        lua_getfield (L, -1, "x");
        vecValue.x = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        vecValue.y = lua_tonumber(L, -1);
        lua_pop(L, 1);
        GetVariable(comp, name, glm::vec2) = vecValue;
    }
    else if(std::holds_alternative<glm::vec3>(var.Contents)) {
        glm::vec3& vecValue = std::get<glm::vec3>(var.Contents);
        lua_getfield (L, -1, "x");
        vecValue.x = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        vecValue.y = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "z");
        vecValue.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
        GetVariable(comp, name, glm::vec3) = vecValue;
    }
    else if(std::holds_alternative<glm::vec4>(var.Contents)) {
        glm::vec4& vecValue = std::get<glm::vec4>(var.Contents);
        lua_getfield (L, -1, "x");
        vecValue.x = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        vecValue.y = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "z");
        vecValue.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "w");
        vecValue.z = lua_tonumber(L, -1);
        lua_pop(L, 1);
        GetVariable(comp, name, glm::vec4) = vecValue;
    }
}
Component::Component(std::string File,std::string ArgName, unsigned int ArgId, bool luaComp) :LuaFile(File), Name(ArgName), id(ArgId)
{
    if(!luaComp) return;
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "SapphireEngine", LuaUtilities::luaopen_SapphireEngine, 0);

    //Setting up the component to access from lua.
    auto ComponentIndex = [](lua_State* L) -> int
    {
        Component* comp = (Component*)lua_touserdata(L, -2);
        const char* index = lua_tostring(L, -1);

        for(auto &var : comp->Variables)
        {
            if (strcmp(index, var.first.c_str()) == 0)
            {
                GET_VAR_FROM_LUA(L, var.second);
                return 1;
            }
        }
        delete(index);
        delete(comp);
        return 0;
    };
    auto ComponentNewIndex = [](lua_State* L) -> int
    {
        Component* comp = (Component*)lua_touserdata(L, -3);
        const char* index = lua_tostring(L, -2);
        const char*  test = lua_tostring(L, -1);
        for(auto &var : comp->Variables)
        {
            if (strcmp(index, var.first.c_str()) == 0)
            {
                SET_VAR_FROM_LUA(L, comp, var.second, std::string(index));
                return 0;
            }
        }
        delete(index);
        delete(comp);
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

void Component::ExecuteFunction(std::string Name)
{
    std::string FullPath = LuaFile;
    UpdateLuaVariables();
    ScriptingEngine::LuaFunction(L, Name);
}
//! This could be organized a bit more
void Component::UpdateLuaVariables()
{
    if(Variables.size() == 0) return;
    for(auto const& Var : Variables)
    {
        if(Var.second.Type == LUA_TNUMBER){
            float val = (std::get<float>(Var.second.Contents));
            lua_pushnumber(L, val);
        }
        if(Var.second.Type == LUA_TSTRING){
            lua_pushstring(L, std::get<std::string>(Var.second.Contents).c_str());
        }if(Var.second.Type == LUA_TBOOLEAN){
            lua_pushboolean(L, std::get<bool>(Var.second.Contents) == true);
        }
        if(Var.second.Type == LUA_TTABLE)
        {
            lua_newtable(L);
            for(const LuaTableIt& TableVariable : std::get<std::vector<LuaTableIt>>(Var.second.Contents))
            {
                if(TableVariable.Type == LUA_TSTRING){
                    if(std::holds_alternative<int>(TableVariable.Key)){
                        lua_pushstring(L, std::to_string(std::get<int>(TableVariable.Key)).c_str());
                    }else if(std::holds_alternative<std::string>(TableVariable.Key)){
                        lua_pushstring(L, std::get<std::string>(TableVariable.Key).c_str());
                    }
                    lua_pushstring(L, std::get<std::string>(TableVariable.Contents).c_str());
                    lua_settable(L, -3);
                }else if(TableVariable.Type == LUA_TNUMBER){ 
                    if(std::holds_alternative<int>(TableVariable.Key)){
                        lua_pushstring(L, std::to_string(std::get<int>(TableVariable.Key)).c_str());
                    }else if(std::holds_alternative<std::string>(TableVariable.Key)){
                        lua_pushstring(L, std::get<std::string>(TableVariable.Key).c_str());
                    }
                    lua_pushnumber(L, std::get<float>(TableVariable.Contents));
                    lua_settable(L, -3);
                }else if(TableVariable.Type == LUA_TBOOLEAN){
                    if(std::holds_alternative<int>(TableVariable.Key)){
                        lua_pushstring(L, std::to_string(std::get<int>(TableVariable.Key)).c_str());
                    }else if(std::holds_alternative<std::string>(TableVariable.Key)){
                        lua_pushstring(L, std::get<std::string>(TableVariable.Key).c_str());
                    }
                    lua_pushboolean(L, std::get<bool>(TableVariable.Contents));
                    lua_settable(L, -3);
                }
            }
        }
        lua_setglobal(L, Var.first.c_str());
    }
}
bool isKnownModule(lua_State* L, std::string name) {
    // These are all the tables included by the lua_openlibs() function and yes that was the best way i found
    return name != "_G" && name != "package" && name != "package.loaded" && name != "package.preload" && name != "coroutine" && name != "string" && name != "table" && name != "math" && name != "io" && name != "os" && name != "debug" && name != "utf8";
}
bool Component::GetLuaVariables()
{
    if (luaL_loadfile(L, LuaFile.c_str()) || lua_pcall(L, 0, 0, 0)) {
        std::stringstream ss;
        ss<< "Error loading script: " << lua_tostring(L, -1) << std::endl;
        Utilities::Log(ss.str(), Utilities::Error);
        lua_pop(L, 1);
        return false;
    }
    lua_getglobal(L, "_G"); // All global variables inside the lua state
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        std::pair<std::string, Variable> var;
        var.first = std::string(lua_tostring(L, -2));
        lua_getglobal(L, var.first.c_str());
        var.second.Type = lua_type(L, -2);
        const char* typeName = lua_typename(L, var.second.Type);
        lua_pop(L, 1);
        if (var.first.c_str() && !lua_isfunction(L, -1)) {
            if(var.second.Type == LUA_TTABLE && isKnownModule(L, var.first.c_str())){
                std::vector<LuaTableIt> test = ScriptingEngine::GetTable(L, std::string(var.first), {});
                var.second.Contents = test;
                Variables[var.first] = var.second;
                lua_pop(L, 1);
                continue;
            }
            const char* VarValue = lua_tostring(L, -1); // Here this variable helps me to decide whether the variable is from the user and not from lua's packages
            //Checking up here because lua_tostring(L, -1) returns 0x0 for false and the if statement returns false and doesn't register the variable
            if(var.second.Type == LUA_TBOOLEAN){
                var.second.Contents = !(VarValue == nullptr);
                Variables[var.first] = var.second;
                lua_pop(L, 1);
                continue;
            }

            //Also checking for if the name is == to "_VERSION" because the _G table also contains the lua version and its not necessary to be displayed.
            if (VarValue && var.first != "_VERSION") {
                if(var.second.Type == LUA_TSTRING){
                    var.second.Contents = lua_tostring(L, -1);
                }else if(var.second.Type == LUA_TNUMBER){
                    std::get<float>(var.second.Contents) = lua_tonumber(L, -1);
                }
                Variables[var.first] = var.second;
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
    
    lua_pushlightuserdata(ComponentsState, this);

    luaL_getmetatable(ComponentsState, "Component");
    lua_setmetatable(ComponentsState, -2);

    lua_setfield(ComponentsState, -2, Name.c_str());
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
        VariantHolds(float, Var.second.Contents, JSONVariables[Var.first] = {Var.second.Type, std::get<float>(Var.second.Contents)})
        VariantHolds(std::string, Var.second.Contents, JSONVariables[Var.first] = {Var.second.Type, std::get<std::string>(Var.second.Contents)})
        VariantHolds(bool, Var.second.Contents, JSONVariables[Var.first] = {Var.second.Type, std::get<bool>(Var.second.Contents)})
        VariantHolds(std::vector<LuaTableIt>, Var.second.Contents, JSONVariables[Var.first] = {Var.second.Type, SaveTable(std::get<std::vector<LuaTableIt>>(Var.second.Contents))})
    }
    return JSONVariables;
}
void RenderTable(std::string text, std::vector<LuaTableIt> &Var){
    if(ImGui::TreeNode(text.c_str()))
    {
        for(auto& TableVariable : Var)
        {
            if(TableVariable.Type == LUA_TSTRING){ ImGui::InputText(std::get<std::string>(TableVariable.Key).c_str(), const_cast<char*>(std::get<std::string>(TableVariable.Contents).c_str()), sizeof(char) * 132);}
            if(TableVariable.Type == LUA_TNUMBER){ ImGui::InputFloat(std::get<std::string>(TableVariable.Key).c_str(), &std::get<float>(TableVariable.Contents));}
            if(TableVariable.Type == LUA_TNUMBER){ ImGui::Checkbox(std::get<std::string>(TableVariable.Key).c_str(), &std::get<bool>(TableVariable.Contents));}
            if(TableVariable.Type == LUA_TNUMBER){ RenderTable(std::get<std::string>(TableVariable.Key), std::get<std::vector<LuaTableIt>>(TableVariable.Contents));}
        }
        ImGui::TreePop();
    }
}
std::vector<LuaTableIt> LoadTable(nlohmann::json Var){
    std::vector<LuaTableIt> Table;
    for(auto& TableVariable : Var.items()){
        LuaTableIt TableVar;
        TableVar.Key = TableVariable.key();
        TableVar.Type = TableVariable.value()[0];
        if(TableVariable.value()[0] == LUA_TNUMBER) {TableVar.Contents = TableVariable.value()[1].get<float>();}
        if(TableVariable.value()[0] == LUA_TSTRING) {TableVar.Contents = TableVariable.value()[1].get<std::string>();}
        if(TableVariable.value()[0] == LUA_TBOOLEAN) {TableVar.Contents = TableVariable.value()[1].get<bool>();}
        if(TableVariable.value()[0] == LUA_TTABLE) {TableVar.Contents = LoadTable(TableVariable.value()[1]);}
        Table.push_back(TableVar);
    }
    return Table;
}
void Component::Render()
{
    ImGui::Separator();
    ImGui::Text(Name.c_str());
    for (auto& Var : Variables)
    {
        VariantHolds(float, Var.second.Contents, ImGui::InputFloat(Var.first.c_str(), &std::get<float>(Var.second.Contents)))
        VariantHolds(bool, Var.second.Contents, ImGui::Checkbox(Var.first.c_str(), &std::get<bool>(Var.second.Contents)))
        VariantHolds(std::string, Var.second.Contents, ImGui::InputText(Var.first.c_str(), const_cast<char*>(std::get<std::string>(Var.second.Contents).c_str()), sizeof(char) * 128));
        VariantHolds(glm::vec2, Var.second.Contents, ImGui::InputFloat2(Var.first.c_str(), &std::get<glm::vec2>(Var.second.Contents).x));
        VariantHolds(glm::vec3, Var.second.Contents, ImGui::InputFloat3(Var.first.c_str(), &std::get<glm::vec3>(Var.second.Contents).x));
        VariantHolds(glm::vec4, Var.second.Contents, ImGui::ColorEdit4(Var.first.c_str(), &std::get<glm::vec4>(Var.second.Contents).x));
        VariantHolds(std::vector<LuaTableIt>, Var.second.Contents, 
            RenderTable(Var.first.c_str(), std::get<std::vector<LuaTableIt>>(Var.second.Contents))
        )
    }
}
bool is2DVector(nlohmann::json&& jsonObject) {
    return jsonObject.contains("x") && jsonObject.contains("y");
}
bool is3DVector(nlohmann::json&& jsonObject) {
    return is2DVector(std::move(jsonObject)) && jsonObject.contains("z");
}
bool is4DVector(nlohmann::json&& jsonObject) {
    return is3DVector(std::move(jsonObject)) && jsonObject.contains("w");
}
void Component::Load(nlohmann::json JSON){
    for(auto& Var : JSON.items()){
        //TODO change it to lua comparisons for example if() jsonArray[0] == LUA_TNUMBER ){...}. So I can implement a number vector.
        //TODO now that i think about it templates would suit this better.
        std::pair<std::string, Variable> NewVar;
        NewVar.first = Var.key();
        nlohmann::json& jsonArray = Var.value();
        // Check if the value is a JSON object with "x", "y", and "z" keys.
        // I prefer having the if statments as one liners when I just need to call one function.
        if (is4DVector(std::move(jsonArray[1]))) { AssignValue<glm::vec4>(NewVar.second, jsonArray);} 
        else if (is3DVector(std::move(jsonArray[1]))) { AssignValue<glm::vec3>(NewVar.second, jsonArray);} 
        else if (is2DVector(std::move(jsonArray[1]))) { AssignValue<glm::vec2>(NewVar.second, jsonArray);} 
        else if(jsonArray[1].is_object()){ NewVar.second.Contents = LoadTable(jsonArray[1]); }
        else if(jsonArray[1].is_number_float()){ AssignValue<float>(NewVar.second, jsonArray); } 
        else if(jsonArray[1].is_string()){ AssignValue<std::string>(NewVar.second, jsonArray); }
        else if(jsonArray[1].is_boolean()){ AssignValue<bool>(NewVar.second, jsonArray); }
        else {Utilities::Log(std::string(jsonArray[1].type_name()) + " cannot be loaded from the selected scene.", Utilities::Error);}
        Variables[NewVar.first] = NewVar.second;
    }
}

void Renderer::Render(Object* Obj,GLFWwindow* window, glm::vec3 CameraPos, std::shared_ptr<Object> SelectedObj, bool IsViewport)
{
    //That means that the object is an empty
    if(shape == nullptr) return;
    // Here it renders the object's outline to indicate that the current object is selected
    if(SelectedObj.get() == Obj){
        shape->Render(GetVariable(Obj->GetComponent<Transform>(), "Size", glm::vec2), GetVariable(this, "Color", glm::vec4), GetVariable(Obj->GetComponent<Transform>(), "Position", glm::vec3), GetVariable(Obj->GetComponent<Transform>(), "Rotation", glm::vec3).z, CameraPos ,true, false, IsViewport);
    }
    shape->Render(GetVariable(Obj->GetComponent<Transform>(), "Size", glm::vec2), GetVariable(this, "Color", glm::vec4), GetVariable(Obj->GetComponent<Transform>(), "Position", glm::vec3), GetVariable(Obj->GetComponent<Transform>(), "Rotation", glm::vec3).z, CameraPos ,false, false, IsViewport);
}
