#include "Engine/Scenes.h"
#include "json.hpp"
#include "UI/FileExplorer/FileExplorer.h"
#include "Objects.h"
#include "Engine/Engine.h"

Object::Object(std::string &&Name)
{
    this->Name = Name;
}
void Object::RemoveComponent(unsigned int id)
{
    Components.erase(Components.begin() + id);
}

void Object::SetUpObject(Object *obj, lua_State *L, const std::string& Name)
{
    lua_pushlightuserdata(L, obj);
    luaL_getmetatable(L, "ObjectMetaTable");
    lua_istable(L, -1);
    lua_setmetatable(L, -2);
    lua_setglobal(L, Name.c_str());
}
//! CREATE A MACRO TO REDUCE THIS REPETITIVE CODE
void Object::OnCollision(Object *other)
{
    for (size_t i = 0; i < Components.size(); i++)
    {
        if (!Components[i]->Active || Components[i]->GetFile().empty())
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
        if (!Components[i]->Active || Components[i]->GetFile().empty())
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
        if (!Components[i]->Active || Components[i]->GetFile().empty())
            continue;
        Components[i]->ExecuteFunction("OnUpdate");
    }
}

std::shared_ptr<Object> Object::CreateObject(std::string &&ObjName)
{
    std::shared_ptr<Object> NewObj = std::make_shared<Object>(std::move(ObjName));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Transform>("", "Transform", 0, false)));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Renderer>("", "Renderer",0, false)));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<RigidBody>("", "Rigidbody", 0, false)));

    NewObj->renderer = NewObj->GetComponent<Renderer>(); 
    NewObj->transform = NewObj->GetComponent<Transform>(); 

    NewObj->renderer->Color.AnyValue() = glm::vec4(1);
    NewObj->transform->Position.AnyValue() = glm::vec3(0);
    NewObj->transform->Size.AnyValue() = glm::vec3(20.0f, 20.0f, 0.0f);
    Engine::Get().GetActiveScene()->Objects.push_back(NewObj);


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
    if(ImGui::Button("Create Prefab")){
        SavePrefab();
    }

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

void Object::SavePrefab()
{
    std::ofstream stream(Engine::Get().GetMainPath() + SapphireEngine::Replace(this->Name, ' ', '_') + ".obj");

    nlohmann::json JsonObj;

    nlohmann::json JsonComponents;
    JsonObj["Name"] = this->Name;
    for (auto &component : this->GetComponents())
    {
        nlohmann::json JsonComp;
        JsonComp["path"] = component->GetFile();
        JsonComp["Variables"] = component->Save(); // component->Save returns a json with all the variables
        JsonComponents[component->Name] = JsonComp;
    }
    JsonObj["Components"] = JsonComponents;
    std::stringstream ss;
    JsonObj["shape"] = this->GetComponent<Renderer>()->shape->ShapeType;
    
    stream << JsonObj.dump(2);
    stream.close();
}

std::shared_ptr<Object> Object::LoadPrefab(std::string FilePath)
{
    std::ifstream stream(Engine::Get().GetMainPath() + FilePath, std::ios::binary);
    nlohmann::json JsonObj;
    stream >> JsonObj;
    stream.close();

    std::shared_ptr<Shapes::Shape> shape;
    std::shared_ptr<Object> object = std::make_shared<Object>(JsonObj["Name"]);
    switch (JsonObj["shape"].get<int>())
    {
        case Shapes::CircleT:
            shape = std::make_shared<Shapes::Circle>(Shapes::CircleShader, std::shared_ptr<Object>(object));
            break;
        case Shapes::RectangleT:
            shape = std::make_shared<Shapes::Rectangle>(Shapes::BasicShader, std::shared_ptr<Object>(object));
            break;
        default:
            shape = nullptr;
    }
    nlohmann::json& JsonComp = JsonObj["Components"];
    for (auto& element : JsonObj["Components"].items()) {
        //! Got to find a better way to handle object!
        if(element.key() == "Renderer")
        {
            Renderer* comp = new Renderer(element.value()["path"], element.key(), object->GetComponents().size(), element.value()["path"] != "");
            object->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Renderer>(dynamic_cast<Renderer*>(comp))));
            object->GetComponents().back()->Load(element.value()["Variables"]);
        }
        else if(element.key() == "Transform")
        {
            Transform* comp = new Transform(element.value()["path"], element.key(), object->GetComponents().size(), element.value()["path"] != "");
            object->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Transform>(dynamic_cast<Transform*>(comp))));
            object->GetComponents().back()->Load(element.value()["Variables"]);
        }else
        {
            Component* comp = new Component(element.value()["path"], element.key(), object->GetComponents().size(), element.value()["path"] != "");
            comp->Load(element.value()["Variables"]);
            object->AddComponent<Component>(comp);
        }
    }

    object->GetComponent<Renderer>()->shape = shape;
    Engine::Get().GetActiveScene()->Objects.push_back(object);
    return object;
}