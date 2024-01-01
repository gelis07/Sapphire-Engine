#include "Scenes.h"
#include "json.hpp"
#include <vector>
#include "Engine.h"
#include "Editor.h"
#include "Editor/UI/FileExplorer/FileExplorer.h"
ObjectRef Scene::Add(Object &&obj)
{
    Objects.push_back(obj);
    int refID = SapphireEngine::RandomNumber(1,100000);
    Objects.back().SetRefID(refID);
    ObjectRefrences.emplace(refID, Objects.size()-1);
    return Objects.back().GetRef();
}
ObjectRef Scene::Add(Object &&obj, int refID)
{
    Objects.push_back(obj);
    Objects.back().SetRefID(refID);
    ObjectRefrences.emplace(refID, Objects.size()-1);
    return Objects.back().GetRef();
}
void Scene::Delete(int ID)
{
    ObjectRefrences.erase(ObjectRefrences.find((Objects.begin() + ID)->GetRefID()));
    for (auto& ref : ObjectRefrences)
    {
        if(ref.second < ID) continue;
        Objects[ObjectRefrences[ref.first]].id--;
        ref.second--;
    }
    Objects.erase(Objects.begin() + ID);
}
void Scene::Delete(Object *obj)
{
    const int& ID = ObjectRefrences[obj->GetRefID()];
    ObjectRefrences.erase(ObjectRefrences.find(obj->GetRefID()));
    for (auto& ref : ObjectRefrences)
    {
        if(ref.second < ID) continue;
        Objects[ObjectRefrences[ref.first]].id--;
        ref.second--;
    }
    Objects.erase(Objects.begin() + ID);
}
void Scene::DeleteRuntime(Object *obj)
{
    const int& ID = ObjectRefrences[obj->GetRefID()];
    ObjectsToDelete.push_back(ID);
}
nlohmann::ordered_json SaveObject(Object &obj)
{
    nlohmann::ordered_json JsonObj;
    nlohmann::ordered_json JsonComponents;
    JsonObj["Name"] = obj.Name;
    JsonObj["Tag"] = obj.Tag;
    JsonObj["ID"] = obj.GetRefID();
    
    for (auto &component : obj.GetComponents())
    {
        nlohmann::ordered_json JsonComp;
        JsonComp["path"] = component->GetFile();
        JsonComp["Variables"] = component->Save(); // component->Save returns a json with all the variables
        JsonComponents[component->Name] = JsonComp;
    }
    nlohmann::ordered_json JsonChildren;
    for (size_t i = 0; i < obj.Children.size(); i++)
    {
        std::stringstream ss;
        ss << "Child: " << i;
        JsonChildren[ss.str().c_str()] = SaveObject(*obj.Children[i].Get());
    }
    JsonObj["Children"] = JsonChildren;
    JsonObj["Components"] = JsonComponents;
    if(obj.GetComponent<Renderer>() != nullptr)
        JsonObj["shape"] = obj.GetComponent<Renderer>()->shape->ShapeType;
    return JsonObj;
}
void Scene::Save(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ofstream stream(Engine::GetMainPath() + FilePath, std::ofstream::trunc);
    nlohmann::ordered_json Data;
    unsigned int i = 0;
    for (auto &obj : Objects)
    {
        if(obj.Parent != null_ref) continue;
        std::stringstream ss;
        ss << "Object: " << i;
        Data[ss.str().c_str()] = SaveObject(obj);
        i++;
    }
    stream << Data.dump(2);
    stream.close();
}
Object Scene::LoadObj(nlohmann::ordered_json& JsonObj, int i, std::vector<ObjectRef>& o_CreatedChildren){
    Object obj(JsonObj["Name"]);
    obj.Tag = JsonObj["Tag"];
    if(obj.Tag != "" && std::find(Engine::Tags.begin(), Engine::Tags.end(), obj.Tag) == Engine::Tags.end()){
        Engine::Tags.push_back(obj.Tag);
    }
    std::shared_ptr<SapphireRenderer::Shape> shape = nullptr;
    nlohmann::ordered_json &JsonComp = JsonObj["Components"];
    for (auto &element : JsonObj["Components"].items())
    {
        //! Got to find a better way to handle this!
        if (element.key() == "Renderer")
        {
            obj.GetComponents().push_back(std::static_pointer_cast<Component>(std::make_shared<Renderer>(element.value()["path"], element.key(), obj.GetComponents().size(), element.value()["path"] != "")));
            obj.GetComponents().back()->Load(element.value()["Variables"]);
        }
        else if (element.key() == "Transform")
        {
            std::vector<glm::vec3> points;
            points.push_back(glm::vec3(-0.5f, -0.5f, 0));
            points.push_back(glm::vec3(0.5f, -0.5f, 0));
            points.push_back(glm::vec3(0.5f, 0.5f, 0));
            points.push_back(glm::vec3(-0.5f, 0.5f, 0));
            obj.GetComponents().push_back(std::make_shared<Transform>(element.value()["path"], element.key(), obj.GetComponents().size(), std::move(points), element.value()["path"] != ""));
            obj.GetComponents().back()->Load(element.value()["Variables"]);
        }
        else if (element.key() == "Camera")
        {
            obj.GetComponents().push_back(std::make_shared<LuaCamera>(element.value()["path"], element.key(), obj.GetComponents().size(), element.value()["path"] != ""));
            obj.GetComponents().back()->Load(element.value()["Variables"]);
            Engine::CameraObjectID = i;
        }
        else if (element.key() == "Rigidbody")
        {
            obj.GetComponents().push_back(std::make_shared<SapphirePhysics::RigidBody>(element.value()["path"], element.key(), obj.GetComponents().size(), element.value()["path"] != ""));
            obj.GetComponents().back()->Load(element.value()["Variables"]);
        }
        else
        {
            std::shared_ptr<Component> comp = std::make_shared<Component>(element.value()["path"], element.key(), obj.GetComponents().size(), element.value()["path"] != "");
            comp->Load(element.value()["Variables"]);
            obj.AddComponent<Component>(comp);
        }
    }

    obj.GetTransform() = obj.GetComponent<Transform>();
    obj.GetTransform()->UpdateModel();
    obj.GetTransform()->UpdatePoints();
    obj.GetRenderer() = obj.GetComponent<Renderer>();
    obj.GetRb() = obj.GetComponent<SapphirePhysics::RigidBody>();

    for(auto& child : JsonObj["Children"].items()){
        std::vector<ObjectRef> children; //This represents the children that may be inside the child
        Object childObj = LoadObj(child.value(), i, children);
        ObjectRef NewChildObj = Add(std::move(childObj));
        obj.Children.push_back(NewChildObj);
        o_CreatedChildren.push_back(NewChildObj);
        NewChildObj->GetTransform()->Parent = obj.GetTransform().get();
        obj.GetTransform()->childrenTransforms.push_back(obj.Children.back()->GetTransform().get());
        obj.GetTransform()->UpdateModel();
        //If the NewObj that was just loaded has any children, set their parent to be the new object.
        for (auto &&newChild : children)
        {
            newChild->Parent = NewChildObj;
        }
    }
    if(JsonObj.find("shape") != JsonObj.end() && obj.GetComponent<Renderer>() != nullptr){
        switch (JsonObj["shape"].get<int>())
        {
            case SapphireRenderer::CircleT:
            {
                shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices);
                shape->ShapeType = SapphireRenderer::CircleT;
                break;
            }
            case SapphireRenderer::RectangleT:
            {
                shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::TextureShader, SapphireRenderer::RectangleVertices, obj.GetComponent<Renderer>()->TexturePath.Get());
                shape->ShapeType = SapphireRenderer::RectangleT;
                break;
            }
            default:
                shape = nullptr;
        }
    }

    if (i == Engine::CameraObjectID)
    {
        obj.GetComponent<Transform>()->SetSize(glm::vec3(Editor::GetWindowSize().x, Editor::GetWindowSize().y, 0));
        shape->Wireframe() = true;
    }
    if(obj.GetComponent<Renderer>() != nullptr){
        if (obj.GetComponent<Renderer>()->TexturePath.Get() != "")
        {
            shape->Load(Engine::GetMainPath() + obj.GetComponent<Renderer>()->TexturePath.Get(), true);
        }
        obj.GetComponent<Renderer>()->shape = shape;
    }

    if (std::shared_ptr<SapphirePhysics::RigidBody> RbComp = obj.GetComponent<SapphirePhysics::RigidBody>())
    {
        RbComp->transform = obj.GetTransform().get();
        if(obj.GetComponent<Renderer>() != nullptr)
            RbComp->ShapeType = static_cast<int>(obj.GetRenderer()->shape->ShapeType);
    }
    return obj;
}
void Scene::Load(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ifstream stream(Engine::GetMainPath() + FilePath);
    nlohmann::ordered_json Data;
    stream >> Data;
    stream.close();
    for (size_t i = 0; i < Objects.size(); i++)
    {
        ObjectRefrences.erase(ObjectRefrences.find((Objects.begin() + i)->GetRefID()));
    }
    Objects.clear();
    for (size_t i = 0; i < Data.size(); i++)
    {
        std::stringstream ss;
        ss << "Object: " << i;
        nlohmann::ordered_json JsonObj = Data[ss.str().c_str()];
        std::vector<ObjectRef> children;
        ObjectRef NewObj = Add(std::move(LoadObj(JsonObj, i, children)), JsonObj["ID"]);
        //If the NewObj that was just loaded has any children, set their parent to be the new object.
        for (auto &child : children)
        {
            child->Parent = NewObj; 
        }
    }
}

void Scene::Hierechy(Object *SelectedObj, int &SelectedObjID)
{
    if (!(*Editor::GetWindowState("Hierachy")))
        return;
    ImGui::Begin("Hierachy", Editor::GetWindowState("Hierachy"));
    for (size_t i = 0; i < Objects.size(); i++)
    {
        if(Objects[i].Parent != null_ref) continue;
        std::string Name = "";
        Name = Objects[i].Name.c_str();
        if (Name.empty())
        {
            Name = "##";
        }
        if(Objects[i].Children.size() == 0){
            if (ImGui::Selectable((Name + "##" + std::to_string(Objects[i].id)).c_str(), &Objects[i] == SelectedObj))
            {
                SelectedObjID = i;

            }
            if(ImGui::IsItemClicked(0)){
                HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(Objects[i].GetRefID()));
            }
        }else{
            if(ImGui::TreeNode((Name + "##" + std::to_string(Objects[i].id)).c_str())){
                if(ImGui::IsItemClicked(0)){
                    SelectedObjID = i;
                }
                for (size_t j = 0; j < Objects[i].Children.size(); j++)
                {
                    if (ImGui::Selectable((Objects[i].Children[j]->Name + "##" + std::to_string(Objects[i].Children[j]->id)).c_str(), &Objects[i] == SelectedObj))
                    {
                        SelectedObjID = i;
                        Editor::SelectedObjChildID = j;
                    }
                }
                ImGui::TreePop();
            }
        }
    }
    if (Engine::app->GetInputDown(GLFW_KEY_DELETE) && ImGui::IsWindowFocused())
    {
        for (size_t i = 0; i < Objects.size(); i++)
        {
            if (Objects[i].Name == SelectedObj->Name)
            {
                Object::Delete(i);
                if(Editor::SelectedObjID >= Objects.size()){
                    Editor::SelectedObjID = Objects.size() - 1;
                }else if(Objects.size() == 0){
                    Editor::SelectedObjID = -1;
                }
            }
        }
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context Menu");
    }
    CreateMenu(SelectedObj);
    ImGui::End();
}

void Scene::CreateMenu(Object *SelectedObj)
{
    if (ImGui::BeginPopup("Context Menu"))
    {
        if (ImGui::MenuItem("Create Empty"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object::CreateObject(ss.str());
        }
        if (ImGui::MenuItem("Create Circle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object *Obj = Object::CreateObject(ss.str());
            Obj->GetComponent<Renderer>()->shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices);
            Obj->GetComponent<Renderer>()->shape->ShapeType = SapphireRenderer::CircleT;
            Obj->GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(Obj->GetRenderer()->shape->ShapeType);
        }
        if (ImGui::MenuItem("Create Rectangle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object *Obj = Object::CreateObject(ss.str());
            Obj->GetComponent<Renderer>()->shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices);
            Obj->GetComponent<Renderer>()->shape->ShapeType = SapphireRenderer::RectangleT;
            Obj->GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(Obj->GetRenderer()->shape->ShapeType);
        }
        if (ImGui::MenuItem("Create Sprite"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object *Obj = Object::CreateObject(ss.str());
            Obj->GetComponent<Renderer>()->shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::TextureShader, SapphireRenderer::RectangleVertices, "");
            Obj->GetComponent<Renderer>()->shape->ShapeType = SapphireRenderer::RectangleT;
            Obj->GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(Obj->GetRenderer()->shape->ShapeType);
        }
        ImGui::EndPopup();
    }
}
