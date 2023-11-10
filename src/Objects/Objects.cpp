#include "Engine/Scenes.h"
#include "json.hpp"
#include "UI/FileExplorer/FileExplorer.h"
#include "Engine/Engine.h"
#include "Objects.h"
#include "RunTime/RunTime.h"


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
        if(RunTime::SkipFrame)
            break;
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

std::shared_ptr<Object> Object::CreateObject(std::string &&ObjName)
{
    std::shared_ptr<Object> NewObj = std::make_shared<Object>(std::move(ObjName));
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,0.5f,0));
    points.push_back(glm::vec3(-0.5f,0.5f,0));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Transform>("", "Transform", 0,NewObj.get(),std::move(points), false)));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<Renderer>("", "Renderer",0, NewObj.get(),false)));
    NewObj->Components.push_back(std::static_pointer_cast<Component>(std::make_shared<SapphirePhysics::RigidBody>("", "Rigidbody", 0,NewObj.get(), false)));

    NewObj->renderer = NewObj->GetComponent<Renderer>(); 
    NewObj->transform = NewObj->GetComponent<Transform>(); 
    
    NewObj->renderer->Color.Get() = glm::vec4(1);
    NewObj->transform->SetSize(glm::vec3(0));
    NewObj->transform->SetSize(glm::vec3(20.0f, 20.0f, 0.0f));

    NewObj->GetComponent<SapphirePhysics::RigidBody>()->transform = NewObj->GetTransform().get();

    Engine::Get().GetActiveScene()->Objects.push_back(NewObj);


    return NewObj;
}

void Object::Delete(int id)
{
    Engine::Get().GetActiveScene()->Objects.erase(Engine::Get().GetActiveScene()->Objects.begin() + id);
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
    if(!(*Engine::Get().GetWindows().GetWindowState("Inspector"))) return;

    ImGui::Begin("Inspect", Engine::Get().GetWindows().GetWindowState("Inspector"));

    ImGui::InputText("Object Name", &Name);
    if(ImGui::Button("Create Prefab")){
        SavePrefab();
    }

    RenderGUI();

    std::shared_ptr<File> *File = FileExplorerDrop.ReceiveDrop(ImGui::GetCurrentWindow());
    if (File != NULL)
    {
        if(File->get()->Extension == ".lua"){
            Component *NewComponent = new Component((*File)->Path, std::string((*File)->Name).erase((*File)->Name.size() - 4, (*File)->Name.size()), Components.size(),this,true);
            if (NewComponent->GetLuaVariables(this))
                AddComponent<Component>(NewComponent);
        }else if(File->get()->Extension == ".png"){
            GetRenderer()->TexturePath.Get() = (*File)->Path;
            GetRenderer()->shape->Load(Engine::Get().GetMainPath() + (*File)->Path, true);
        }else if(File->get()->Extension == ".anim"){
            // std::vector<SapphireRenderer::KeyFramePair> KeyFrames;
            // SapphireRenderer::KeyFramePair test;
            // test.TimeStamp = 0.3f;
            // test.offset = glm::vec2(0,0);
            // KeyFrames.push_back(test);
            // SapphireRenderer::KeyFramePair test2;
            // test2.TimeStamp = 0.6f;
            // test2.offset = glm::vec2(0.5f, 0);
            // KeyFrames.push_back(test2);
            // std::vector<Vertex> Vertices;
            // Vertices.push_back({glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)});
            // Vertices.push_back({glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.0f)});
            // Vertices.push_back({glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f)});
            // Vertices.push_back({glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)});
            // GetRenderer()->shape = std::make_shared<SapphireRenderer::Animation>(SapphireRenderer::Animation::LoadAnim(Engine::Get().GetMainPath() + (*File)->Path),std::move(Vertices),SapphireRenderer::TextureShader, std::string("C:/Gelis/OMORFONN.png"));
            // ((SapphireRenderer::Animation*)GetRenderer()->shape.get())->SaveAnim(Engine::Get().GetMainPath() + "/animation.anim");
        }
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

    std::shared_ptr<SapphireRenderer::Shape> shape;
    std::shared_ptr<Object> object = std::make_shared<Object>(JsonObj["Name"]);
    switch (JsonObj["shape"].get<int>())
    {
        case SapphireRenderer::CircleT:
            shape = std::make_shared<SapphireRenderer::Circle>(SapphireRenderer::CircleShader);
            break;
        case SapphireRenderer::RectangleT:
            shape = std::make_shared<SapphireRenderer::Rectangle>(SapphireRenderer::BasicShader);
            break;
        default:
            shape = nullptr;
    }
    nlohmann::json& JsonComp = JsonObj["Components"];
    for (auto& element : JsonObj["Components"].items()) {
        //! Got to find a better way to handle object!
        if(element.key() == "Renderer")
        {
            Renderer* comp = new Renderer(element.value()["path"], element.key(), object->GetComponents().size(), object.get(),element.value()["path"] != "");
            object->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Renderer>(dynamic_cast<Renderer*>(comp))));
            object->GetComponents().back()->Load(element.value()["Variables"]);
        }
        else if(element.key() == "Transform")
        {
            std::vector<glm::vec3> points;
            points.push_back(glm::vec3(-0.5f,-0.5f,0));
            points.push_back(glm::vec3(0.5f,-0.5f,0));
            points.push_back(glm::vec3(0.5f,0.5f,0));
            points.push_back(glm::vec3(-0.5f,0.5f,0));
            Transform* comp = new Transform(element.value()["path"], element.key(), object->GetComponents().size(), object.get(),std::move(points),element.value()["path"] != "");
            object->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Transform>(dynamic_cast<Transform*>(comp))));
            object->GetComponents().back()->Load(element.value()["Variables"]);
        }else
        {
            Component* comp = new Component(element.value()["path"], element.key(), object->GetComponents().size(), object.get(),element.value()["path"] != "");
            comp->Load(element.value()["Variables"]);
            object->AddComponent<Component>(comp);
        }
    }

    object->GetComponent<Renderer>()->shape = shape;
    Engine::Get().GetActiveScene()->Objects.push_back(object);
    return object;
}