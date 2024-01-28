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
    ObjectRef obj = Objects[ID].GetRef();
    for (auto& child : obj->Children)
    {
        Delete(child.Get());
    }
    ID = ObjectRefrences[obj->GetRefID()];
    ObjectRefrences.erase(obj->GetRefID());
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
    for (auto& child : obj->Children)
    {
        Delete(child.Get());
    }
    int ID = ObjectRefrences[obj->GetRefID()];
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
    if(obj.GetComponent<Renderer>() != nullptr){
        JsonObj["shape"] = obj.GetComponent<Renderer>()->ShapeType;
        nlohmann::ordered_json Animations;
        for (auto &&anim : obj.GetComponent<Renderer>()->Animations)
        {
            Animations[anim.first] = anim.second->GetAnimFile();
        }
        JsonObj["Components"]["Renderer"]["Animations"] = Animations;
    }
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
    nlohmann::ordered_json &JsonComp = JsonObj["Components"];
    for (auto &element : JsonObj["Components"].items())
    {
        if(Component::ComponentTypeRegistry.find(element.key()) != Component::ComponentTypeRegistry.end()){
            Component::ComponentTypeRegistry[element.key()](&obj);
        }
        else{
            std::shared_ptr<Component> comp = std::make_shared<Component>(element.value()["path"], element.key(), obj.GetComponents().size());
            obj.AddComponent<Component>(comp);
        }
        if (element.key() == "Renderer")
        {
            // if(JsonObj.find("shape") != JsonObj.end()){
            //     switch (JsonObj["shape"].get<int>())
            //     {
            //         case SapphireRenderer::CircleT:{
            //             obj.AddComponent(std::make_shared<Renderer>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::CircleT));
            //             obj.GetComponents().back()->Load(element.value()["Variables"]);
            //             break;
            //         }
            //         case SapphireRenderer::RectangleT:{
            //             obj.AddComponent(std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT));
            //             obj.GetComponents().back()->Load(element.value()["Variables"]);
            //             break;
            //         }
            //     }
            // }
            for (auto &anim : element.value()["Animations"].items())
            {
                std::string FullPath = anim.value().get<std::string>();
                std::string FileName = anim.key();
                obj.GetComponent<Renderer>()->Animations.emplace(FileName, new SapphireRenderer::Animation(FullPath,FullPath.substr(0, FullPath.length() - 4) + "png"));
                obj.GetComponent<Renderer>()->SelectAnimation(FileName);
            }
        }
        else if (element.key() == "Camera")
        {
            Engine::CameraObjectID = i;
        }
        obj.GetComponents().back()->Load(element.value()["Variables"]);
    }
    obj.GetComponent<Transform>()->UpdateModel();
    obj.GetComponent<Transform>()->UpdatePoints();
    for(auto& child : JsonObj["Children"].items()){
        std::vector<ObjectRef> children; //This represents the children that may be inside the child
        Object childObj = LoadObj(child.value(), i, children);
        ObjectRef NewChildObj = Add(std::move(childObj), child.value()["ID"]);
        obj.Children.push_back(NewChildObj);
        o_CreatedChildren.push_back(NewChildObj);
        NewChildObj->GetComponent<Transform>()->Parent = obj.GetComponent<Transform>().get();
        obj.GetComponent<Transform>()->childrenTransforms.push_back(obj.Children.back()->GetComponent<Transform>().get());
        obj.GetComponent<Transform>()->UpdateModel();
        //If the NewObj that was just loaded has any children, set their parent to be the new object.
        for (auto &&newChild : children)
        {
            newChild->Parent = NewChildObj;
        }
    }
    if (i == Engine::CameraObjectID)
    {
        obj.GetComponent<Camera>()->Transform = obj.GetComponent<Transform>();
    }
    if(obj.GetComponent<Renderer>() != nullptr){
        if (obj.GetComponent<Renderer>()->TexturePath.Get() != "")
        {
            if(obj.GetComponent<Renderer>()->TexturePath.Get().substr(obj.GetComponent<Renderer>()->TexturePath.Get().length() - 3) == "png"){
                obj.GetComponent<Renderer>()->Load(Engine::GetMainPath() + obj.GetComponent<Renderer>()->TexturePath.Get(), true);
            }else{
                std::string FullPath = Engine::GetMainPath() + obj.GetComponent<Renderer>()->TexturePath.Get();
                obj.GetComponent<Renderer>()->Animations.emplace(obj.GetComponent<Renderer>()->TexturePath.Get().substr(0, obj.GetComponent<Renderer>()->TexturePath.Get().length() - 5), new SapphireRenderer::Animation(FullPath,FullPath.substr(0, FullPath.length() - 4) + "png"));
                obj.GetComponent<Renderer>()->SelectAnimation(obj.GetComponent<Renderer>()->TexturePath.Get().substr(0, obj.GetComponent<Renderer>()->TexturePath.Get().length() - 5));
            }
        }
        obj.GetComponent<Renderer>()->transform = obj.GetComponent<Transform>();
        Renderer::SceneRenderers.push_back(obj.GetComponent<Renderer>());
    }

    if (std::shared_ptr<SapphirePhysics::RigidBody> RbComp = obj.GetComponent<SapphirePhysics::RigidBody>())
    {
        RbComp->transform = obj.GetComponent<Transform>().get();
        if(obj.GetComponent<Renderer>() != nullptr)
            RbComp->ShapeType = static_cast<int>(obj.GetComponent<Renderer>()->ShapeType);

        SapphirePhysics::RigidBody::Rigibodies.push_back(RbComp.get());
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
    Renderer::SceneRenderers.clear();
    Renderer::Gizmos.clear();
    SapphirePhysics::RigidBody::Rigibodies.clear();
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
    std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT);
    rend->Wireframe = true;
    rend->transform = Engine::GetCameraObject()->GetComponent<Transform>();
    Renderer::Gizmos.push_back(rend);
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
        if(Objects[i].Children.size() == 0 && Objects[i].Parent == null_ref){
            if (ImGui::Selectable((Name + "##" + std::to_string(Objects[i].id)).c_str(), &Objects[i] == SelectedObj))
            {
                SelectedObjID = i;
                Editor::SelectedObjChildID = -1;
            }
            ImVec2 textSize = ImGui::CalcTextSize(Name.c_str());
            if(std::shared_ptr<ObjectRef>* ref = HierachyDrop.ReceiveDropLoop(glm::vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y), glm::vec2(textSize.x, textSize.y))){
                if((**ref) != Objects[i].GetRefID()){
                    Objects[i].Children.push_back(**ref);
                    (**ref)->Parent = Objects[i].GetRef();
                    (**ref)->GetComponent<Transform>()->Parent = Objects[i].GetComponent<Transform>().get();
                    Objects[i].GetComponent<Transform>()->childrenTransforms.push_back((**ref)->GetComponent<Transform>().get());
                    (**ref)->GetComponent<Transform>()->UpdateModel();
                }
            }
            if(ImGui::IsItemClicked(0)){
                HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(Objects[i].GetRefID()));
            }
        }else{
            if(ImGui::TreeNode((Name + "##" + std::to_string(Objects[i].id)).c_str())){
                ImVec2 textSize = ImGui::CalcTextSize(Name.c_str());
                if(std::shared_ptr<ObjectRef>* ref = HierachyDrop.ReceiveDropLoop(glm::vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y), glm::vec2(textSize.x, textSize.y))){
                    if((**ref) != Objects[i].GetRefID()){
                        Objects[i].Children.push_back(**ref);
                        (**ref)->Parent = Objects[i].GetRef();
                        (**ref)->GetComponent<Transform>()->Parent = Objects[i].GetComponent<Transform>().get();
                        Objects[i].GetComponent<Transform>()->childrenTransforms.push_back((**ref)->GetComponent<Transform>().get());
                        (**ref)->GetComponent<Transform>()->UpdateModel();
                    }
                }
                if(ImGui::IsItemClicked(0)){
                    SelectedObjID = i;
                    Editor::SelectedObjChildID = -1;
                    HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(Objects[i].GetRefID()));
                }
                for (size_t j = 0; j < Objects[i].Children.size(); j++)
                {
                    if (ImGui::Selectable((Objects[i].Children[j]->Name + "##" + std::to_string(Objects[i].Children[j]->id)).c_str(), &Objects[i] == SelectedObj))
                    {
                        SelectedObjID = i;
                        Editor::SelectedObjChildID = j;
                    }
                    if(ImGui::IsItemClicked(0)){
                        HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(Objects[i].Children[j]));
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
            Object Obj = Object(ss.str());
            Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Circle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object Obj = Object(ss.str());
            Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Rectangle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object Obj = Object(ss.str());
            Obj.AddComponent((std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT)));
            Obj.GetComponent<Renderer>()->transform = Obj.GetComponent<Transform>();
            Renderer::SceneRenderers.push_back(Obj.GetComponent<Renderer>());
            Obj.GetComponent<Renderer>()->Color.Get() = glm::vec4(1);
            Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Sprite"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object Obj = Object(ss.str());
            Obj.GetComponent<Renderer>()->ShapeType = SapphireRenderer::RectangleT;
            Obj.GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(Obj.GetComponent<Renderer>()->ShapeType);
            Add(std::move(Obj));
        }
        ImGui::EndPopup();
    }
}
