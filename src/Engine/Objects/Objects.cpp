#include "Engine/Scenes.h"
#include "json.hpp"
#include "UI/FileExplorer/FileExplorer.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "Objects.h"


Object::Object(std::string &&Name)
{
    this->Name = Name;
}
Object::~Object()
{
    Components.clear();
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
    if (m_CalledStart)
        return;
    for (size_t i = 0; i < Components.size(); i++)
    {
        if (!Components[i]->Active || Components[i]->GetFile().empty())
            continue;
        lua_State *L = Components[i]->GetState();
        if (!ScriptingEngine::CheckLua(L, luaL_dofile(L, ("C:/Gelis/Programs/Flappy_Bird/Assets/" + GetComponents()[i]->GetFile()).c_str())))
        {
            std::stringstream ss;
            ss << "Error loading script: " << lua_tostring(L, -1) << std::endl;
            Log(ss.str(), SapphireEngine::Error);
            lua_pop(L, 1);
        }
        Components[i]->ExecuteFunction("OnStart");
        m_CalledStart = true;
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

Object* Object::CreateObject(std::string &&ObjName)
{
    Object NewObj(std::move(ObjName));
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,0.5f,0));
    points.push_back(glm::vec3(-0.5f,0.5f,0));
    NewObj.Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Transform>("", "Transform", 0,std::move(points), false)));
    NewObj.Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Renderer>("", "Renderer",0, false)));
    NewObj.Components.push_back(std::static_pointer_cast<Component>(std::make_shared<SapphirePhysics::RigidBody>("", "Rigidbody", 0, false)));

    NewObj.renderer = NewObj.GetComponent<Renderer>(); 
    NewObj.transform = NewObj.GetComponent<Transform>();
    
    NewObj.renderer->Color.Get() = glm::vec4(1);
    NewObj.transform->SetSize(glm::vec3(0));
    NewObj.transform->SetSize(glm::vec3(20.0f, 20.0f, 0.0f));

    NewObj.GetComponent<SapphirePhysics::RigidBody>()->transform = NewObj.GetTransform().get();

    Engine::GetActiveScene().Objects.push_back(NewObj);


    return &Engine::GetActiveScene().Objects.back();
}

void Object::Delete(int id)
{
    Engine::GetActiveScene().Objects.erase(Engine::GetActiveScene().Objects.begin() + id);
}

void Object::RenderGUI()
{
    for (size_t i = 0; i < Components.size(); i++)
    {
        Components[i]->Render();
        if(Components[i]->GetState() != nullptr && ImGui::Button(("Remove Component##" + std::to_string(i)).c_str()))
        {
            Components.erase(Components.begin() + i);
        }
        ImGui::Separator();
    }
}
void Object::Inspect()
{
    if(!(*Editor::GetWindowState("Inspector"))) return;

    ImGui::Begin("Inspect", Editor::GetWindowState("Inspector"));

    ImGui::InputText("Object Name", &Name);
    if(ImGui::Button("Create Prefab")){
        SavePrefab();
    }

    RenderGUI();

    std::shared_ptr<File>* File = FileExplorerDrop.ReceiveDrop(ImGui::GetCurrentWindow());
    if (File != NULL)
    {
        if(File->get()->Extension == ".lua"){
            Component *NewComponent = new Component((*File)->Path, std::string((*File)->Name).erase((*File)->Name.size() - 4, (*File)->Name.size()), Components.size(),true);
            // if (NewComponent->GetLuaVariables(this))
            //     AddComponent<Component>(NewComponent);
        }else if(File->get()->Extension == ".png"){
            GetRenderer()->TexturePath.Get() = (*File)->Path;
            GetRenderer()->shape->Load(Engine::GetMainPath() +(*File)->Path, true);
        }else if(File->get()->Extension == ".anim"){
            std::string FullPath = Engine::GetMainPath() + (*File)->Path;
            GetRenderer()->shape = std::make_shared<SapphireRenderer::Animation>(FullPath, SapphireRenderer::TextureShader
            , SapphireRenderer::RectangleVertices, "hi.png");
            GetRenderer()->shape->ShapeType = SapphireRenderer::RectangleT;
            GetRenderer()->shape->Load(FullPath.substr(0, FullPath.length() - 4) + "png", true);
        }
    }
    ImGui::End();
}

void Object::SavePrefab()
{
    std::ofstream stream(Engine::GetMainPath() + SapphireEngine::Replace(this->Name, ' ', '_') + ".obj");

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

Object* Object::LoadPrefab(std::string FilePath)
{
    std::ifstream stream(Engine::GetMainPath() + FilePath, std::ios::binary);
    nlohmann::json JsonObj;
    stream >> JsonObj;
    stream.close();

    std::shared_ptr<SapphireRenderer::Shape> shape;
    Object object(JsonObj["Name"]);
    switch (JsonObj["shape"].get<int>())
    {
        case SapphireRenderer::CircleT:{
            shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices);
            shape->ShapeType = SapphireRenderer::CircleT;
            break;
        }
        case SapphireRenderer::RectangleT:
            shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::BasicShader,SapphireRenderer::RectangleVertices);
            shape->ShapeType = SapphireRenderer::RectangleT;
            break;
        default:
            shape = nullptr;
    }
    nlohmann::json& JsonComp = JsonObj["Components"];
    for (auto& element : JsonObj["Components"].items()) {
        //! Got to find a better way to handle object!
        if(element.key() == "Renderer")
        {
            object.GetComponents().push_back(std::make_shared<Renderer>(element.value()["path"], element.key(), object.GetComponents().size(),element.value()["path"] != ""));
            object.GetComponents().back()->Load(element.value()["Variables"]);
        }
        else if(element.key() == "Transform")
        {
            std::vector<glm::vec3> points;
            points.push_back(glm::vec3(-0.5f,-0.5f,0));
            points.push_back(glm::vec3(0.5f,-0.5f,0));
            points.push_back(glm::vec3(0.5f,0.5f,0));
            points.push_back(glm::vec3(-0.5f,0.5f,0));
            object.GetComponents().push_back(std::make_shared<Transform>(element.value()["path"], element.key(), object.GetComponents().size(), std::move(points),element.value()["path"] != ""));
            object.GetComponents().back()->Load(element.value()["Variables"]);
        }else
        {
            std::shared_ptr<Component> comp = std::make_shared<Component>(element.value()["path"], element.key(), object.GetComponents().size(), element.value()["path"] != "");
            comp->Load(element.value()["Variables"]);
            object.AddComponent<Component>(comp);
        }
    }

    object.GetComponent<Renderer>()->shape = shape;
    Engine::GetActiveScene().Objects.push_back(object);
    return &Engine::GetActiveScene().Objects.back();
}