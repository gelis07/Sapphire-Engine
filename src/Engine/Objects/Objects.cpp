#include "Engine/Scenes.h"
#include "json.hpp"
#include "UI/FileExplorer/FileExplorer.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "Objects.h"


Object::Object(std::string &&Name)
{
    this->Name = Name;
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,0.5f,0));
    points.push_back(glm::vec3(-0.5f,0.5f,0));
    AddComponent((std::make_shared<Transform>("Transform",std::move(points))));
    GetComponent<Transform>()->SetSize(glm::vec3(0));
    GetComponent<Transform>()->SetSize(glm::vec3(1.0f, 1.0f, 0.0f));
}
Object::~Object()
{
    Components.clear();
}
void Object::RemoveComponent(unsigned int id)
{
    for (auto &&comp : ComponentMap)
    {
        if(comp.second == Components[id]){
            ComponentMap.erase(comp.first);
            break;
        }
    }
    Components.erase(Components.begin() + id);
}

void Object::SetUpObject(Object *obj, lua_State *L, const std::string& Name)
{
    ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
    *ud = obj->GetRef();
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
        Components[i]->UpdateExistingVars();
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
        if (!ScriptingEngine::CheckLua(L, luaL_dofile(L, (Engine::GetMainPath() + GetComponents()[i]->GetFile()).c_str())))
        {
            std::stringstream ss;
            ss << "Error loading script: " << lua_tostring(L, -1) << std::endl;
            Log(ss.str(), SapphireEngine::Error);
            lua_pop(L, 1);
        }
        ObjectRef *ud = (ObjectRef *)lua_newuserdata(L, sizeof(ObjectRef));
        *ud = GetRef();
        luaL_newmetatable(L, "ObjectMetaTable");

        lua_pushstring(L, "__index");
        lua_pushcfunction(L, GetComponentFromObject);
        lua_settable(L, -3);

        lua_setmetatable(L, -2);
        lua_setglobal(L, "this");
        Components[i]->ExecuteFunction("OnStart");
        m_CalledStart = true;
    }
}

int Object::SetActive(lua_State *L)
{
    ObjectRef* obj = static_cast<ObjectRef*>(lua_touserdata(L, 1));
    (*obj)->Active = (bool)lua_toboolean(L, 2);
    return 0;
}

void Object::OnUpdate()
{
    ObjectRef obj(RefID);
    // SapphireEngine::Log(obj->GetComponent<Renderer>()->Name);
    for (size_t i = 0; i < obj->Components.size(); i++)
    {
        if (!obj->Components[i]->Active || obj->Components[i]->GetFile().empty())
            continue;
        obj->Components[i]->ExecuteFunction("OnUpdate");
        obj->Components[i]->UpdateExistingVars();
    }
}



void Object::Delete(int id)
{
    Engine::GetActiveScene().Delete(id);
}

void Object::RenderGUI()
{
    for (size_t i = 0; i < Components.size(); i++)
    {
        Components[i]->Render();
        if(ImGui::Button(("Remove Component##" + std::to_string(i)).c_str()))
        {
            if(Components[i]->Name == "Renderer"){
                for (size_t k = 0; k < Renderer::SceneRenderers.size(); k++)
                {
                    if(Renderer::SceneRenderers[k] == Components[i]){
                        Renderer::SceneRenderers.erase(Renderer::SceneRenderers.begin() + k);
                        std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT);
                        rend->Wireframe = true;
                        rend->transform = this->GetComponent<Transform>();
                        Renderer::Gizmos.push_back(rend);
                    }
                }
            }
            RemoveComponent(i);
        }
        ImGui::Separator();
    }
    if(ImGui::Button("Add Component")){
        ImGui::OpenPopup("Components");
    }
    if(ImGui::BeginPopup("Components")){
        // if(ImGui::Button("RigidBody") && GetComponent<SapphirePhysics::RigidBody>() == nullptr){
        //     AddComponent((std::make_shared<SapphirePhysics::RigidBody>(SapphireRenderer::RectangleT)));
        //     GetComponent<SapphirePhysics::RigidBody>()->transform = GetComponent<Transform>().get();
        //     SapphirePhysics::RigidBody::Rigibodies.push_back(GetComponent<SapphirePhysics::RigidBody>().get());
        // }
        if(ImGui::BeginCombo("Comp", "Select Component")){
            for(const auto &CompReg : Component::ComponentTypeRegistry){
                if(ImGui::Selectable(CompReg.first.c_str()))
                {
                    CompReg.second(this);
                    ImGui::ClosePopupsExceptModals();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::EndPopup();
    }
}
std::string TagInput;
void Object::Inspect()
{
    if(!(*Editor::GetWindowState("Inspector"))) return;

    ImGui::Begin("Inspect", Editor::GetWindowState("Inspector"));
    ImGui::Checkbox("##Active", &Active);
    ImGui::SameLine();
    ImGui::InputText("Object Name", &Name);
    ImGui::SameLine();
    if(ImGui::Button("Create Prefab")){
        SavePrefab();
    }
    if(ImGui::Button("Add")){
        ImGui::OpenPopup("Create Tag");
    }
    if(ImGui::BeginCombo("Tag", Tag.c_str())){
        for(const auto &tag : Engine::Tags){
            if(ImGui::Selectable(tag.c_str()))
            {
                Tag = tag;
            }
        }
        ImGui::EndCombo();
    }
    if(ImGui::BeginPopup("Create Tag")){
        ImGui::InputText("Tag Name", &TagInput);
        if(ImGui::Button("Done") || Engine::app->GetInputDown(GLFW_KEY_ENTER)){
            Engine::Tags.push_back(TagInput);
            TagInput = "";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    RenderGUI();
    
    std::shared_ptr<File>* File = FileExplorerDrop.ReceiveDrop(ImGui::GetCurrentWindow());
    if (File != NULL)
    {
        if(File->get()->Extension == ".lua"){
            std::shared_ptr<Component> NewComponent = std::make_shared<Component>((*File)->Path, std::string((*File)->Name).erase((*File)->Name.size() - 4, (*File)->Name.size()), Components.size());
            lua_State* L = NewComponent->GetState();
            if (NewComponent->GetLuaVariables())
                AddComponent<Component>(NewComponent);
        }else if(File->get()->Extension == ".png"){
            GetComponent<Renderer>()->TexturePath.Get() = (*File)->Path;
            GetComponent<Renderer>()->Load(Engine::GetMainPath() +(*File)->Path, true);
        }else if(File->get()->Extension == ".anim"){
            std::string FullPath = Engine::GetMainPath() + (*File)->Path;
            GetComponent<Renderer>()->TexturePath.Get() = (*File)->Path;
            GetComponent<Renderer>()->Animations.emplace(File->get()->Name.substr(0, File->get()->Name.length() - 5), new SapphireRenderer::Animation(FullPath,FullPath.substr(0, FullPath.length() - 4) + "png"));
            GetComponent<Renderer>()->SelectAnimation(File->get()->Name.substr(0, File->get()->Name.length() - 5));
        }
    }
    ImGui::End();
}

int Object::RemoveComponent(lua_State* L)
{
    ObjectRef* obj = static_cast<ObjectRef*>(lua_touserdata(L, 1));
    const char* namec = lua_tostring(L, 2);
    std::string name(namec);
    for (size_t i = 0; i < (*obj)->GetComponents().size(); i++)
    {
        if((*obj)->GetComponents()[i]->Name == name){
            (*obj)->RemoveComponent(i);
        }
    }
    return 0;
}

void Object::SavePrefab()
{
    std::ofstream stream(Engine::GetMainPath() + SapphireEngine::Replace(this->Name, ' ', '_') + ".obj");

    nlohmann::ordered_json JsonObj;

    nlohmann::ordered_json JsonComponents;
    JsonObj["Name"] = this->Name;
    for (auto &component : this->GetComponents())
    {
        nlohmann::ordered_json JsonComp;
        JsonComp["path"] = component->GetFile();
        JsonComp["Variables"] = component->Save(); // component->Save returns a json with all the variables
        JsonComponents[component->Name] = JsonComp;
    }
    JsonObj["Components"] = JsonComponents;
    std::stringstream ss;
    JsonObj["shape"] = this->GetComponent<Renderer>()->ShapeType;
    
    stream << JsonObj.dump(2);
    stream.close();
}

Object* Object::LoadPrefab(std::string FilePath)
{
    std::ifstream stream(Engine::GetMainPath() + FilePath, std::ios::binary);
    nlohmann::ordered_json JsonObj;
    stream >> JsonObj;
    stream.close();

    Object object(JsonObj["Name"]);

    nlohmann::ordered_json& JsonComp = JsonObj["Components"];
    for (auto& element : JsonObj["Components"].items()) {
        //! Got to find a better way to handle object!
        if(element.key() == "Renderer")
        {
            switch (JsonObj["shape"].get<int>())
            {
                case SapphireRenderer::CircleT:{
                    // shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices);
                    object.GetComponents().push_back(std::make_shared<Renderer>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::CircleT));
                    object.GetComponents().back()->Load(element.value()["Variables"]);
                    break;
                }
                case SapphireRenderer::RectangleT:{
                    object.GetComponents().push_back(std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT));
                    object.GetComponents().back()->Load(element.value()["Variables"]);
                    break;
                }
            }
        }
        else if(element.key() == "Transform")
        {
            object.GetComponent<Transform>()->Load(element.value()["Variables"]);
        }else
        {
            std::shared_ptr<Component> comp = std::make_shared<Component>(element.value()["path"], element.key(), object.GetComponents().size());
            comp->Load(element.value()["Variables"]);
            object.AddComponent<Component>(comp);
        }
    }

    Engine::GetActiveScene().Add(std::move(object));
    return &Engine::GetActiveScene().Objects.back();
}


Object *ObjectRef::operator->()
{
    if(ID != -1)
        return &Engine::GetActiveScene().Objects[Engine::GetActiveScene().ObjectRefrences[ID]];
    else{
        return nullptr;
    }
    return nullptr;
}

bool ObjectRef::operator==(int other)
{
    return ID == other;
}
bool ObjectRef::operator!=(int other)
{
    return ID != other;
}

Object *ObjectRef::Get()
{
    if(ID != -1)
        return &Engine::GetActiveScene().Objects[Engine::GetActiveScene().ObjectRefrences[ID]];
    else{
        return nullptr;
    }
    return nullptr;
}
