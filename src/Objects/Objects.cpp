#include "Engine/Scenes.h"
#include "json.hpp"
#include "UI/FileExplorer/FileExplorer.h"
#include "Objects.h"

Object::Object(std::string &&Name)
{
    this->Name = Name;
}



void Object::RemoveComponent(unsigned int id)
{
    Components.erase(Components.begin() + id);
}

void Object::SetUpObject(Object *obj, lua_State *L, std::string Name)
{
    lua_pushlightuserdata(L, obj);
    luaL_getmetatable(L, "ObjectMetaTable");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    lua_setglobal(L, Name.c_str());
}
//! CREATE A MACRO TO REDUCE THIS REPETETIVE CODE
void Object::OnCollision(Object *other)
{
    for (size_t i = 0; i < Components.size(); i++)
    {
        if (!Components[i]->Active || Components[i]->GetFile() == "")
            continue;
        lua_State *L = Components[i]->GetState();
        SetUpObject(other, L, "obj");
        Components[i]->ExecuteFunction("OnCollision");
    }
}

void Object::OnStart()
{
    if (CalledStart)
        return;
    for (size_t i = 0; i < Components.size(); i++)
    {
        if (!Components[i]->Active || Components[i]->GetFile() == "")
            continue;
        lua_State *L = Components[i]->GetState();
        if (!ScriptingEngine::CheckLua(L, luaL_dofile(L, GetComponents()[i]->GetFile().c_str())))
        {
            std::stringstream ss;
            ss << "Error loading script: " << lua_tostring(L, -1) << std::endl;
            Log(ss.str(), SapphireEngine::Error);
            lua_pop(L, 1);
        }
        Components[i]->ExecuteFunction("OnStart");
        CalledStart = true;
    }
}

void Object::OnUpdate()
{
    for (size_t i = 0; i < Components.size(); i++)
    {
        if (!Components[i]->Active || Components[i]->GetFile() == "")
            continue;
        Components[i]->ExecuteFunction("OnUpdate");
    }
}

std::shared_ptr<Object> Object::CreateObject(std::vector<std::shared_ptr<Object>> &Objects, std::string &&ObjName)
{
    std::shared_ptr<Object> NewObj = std::make_shared<Object>(std::move(ObjName));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Transform>("", "Transform", 0, false)));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Renderer>("", "Renderer", 0, false)));

    NewObj->GetComponent<Renderer>()->Color.AnyValue() = glm::vec4(1);
    NewObj->GetComponent<Transform>()->Position.AnyValue() = glm::vec3(0);
    NewObj->GetComponent<Transform>()->Size.AnyValue() = glm::vec3(20.0f, 20.0f, 0.0f);
    Objects.push_back(NewObj);
    return NewObj;
}

void Object::RenderGUI()
{
    for (std::shared_ptr<Component> Component : Components)
    {
        Component->Render();
    }
}
void Object::Inspect()
{
    ImGui::Begin("Inspect");

    ImGui::InputText("Object Name", &Name);
    // if(ImGui::Button("Create Prefab")){
    //     SavePrefab();
    // }

    RenderGUI();

    std::shared_ptr<File> *File = FileExplorerDrop.ReceiveDrop(ImGui::GetCurrentWindow());
    if (File != NULL)
    {
        Component *NewComponent = new Component((*File)->Path, (*File)->Name, Components.size(), true);
        if (NewComponent->GetLuaVariables())
            AddComponent<Component>(NewComponent);
    }

    ImGui::End();
}

void Object::SavePrefab(std::string path)
{
    std::ofstream stream(path + SapphireEngine::Replace(this->Name, ' ', '_') + ".obj");

    nlohmann::json JsonObj;

    stream << JsonObj.dump(2);
    stream.close();
}

void Object::LoadPrefab(std::string path, std::string FilePath, unsigned int ObjectsSize)
{
    std::ifstream stream(path + FilePath, std::ios::binary);
    nlohmann::json JsonObj;
    stream >> JsonObj;
    stream.close();
}