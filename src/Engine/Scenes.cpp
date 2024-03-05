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
    Renderer* rend = obj->GetComponent<Renderer>().get();
    if(rend){
        for (size_t i = 0; i < Renderer::SceneRenderers.size(); i++)
        {
            if(rend == Renderer::SceneRenderers[i].get()){
                Renderer::SceneRenderers.erase(Renderer::SceneRenderers.begin() + i);
            }
        }
    }
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
    Renderer* rend = obj->GetComponent<Renderer>().get();
    if(rend){
        for (size_t i = 0; i < Renderer::SceneRenderers.size(); i++)
        {
            if(rend == Renderer::SceneRenderers[i].get()){
                Renderer::SceneRenderers.erase(Renderer::SceneRenderers.begin() + i);
            }
        }
    }
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
            std::shared_ptr<Component> comp = std::make_shared<Component>(element.value()["path"], element.key(), obj.GetComponents().size(), null_ref);
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
        NewChildObj->GetComponent<Transform>()->TransParent = obj.GetComponent<Transform>().get();
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
    }

    if (std::shared_ptr<SapphirePhysics::RigidBody> RbComp = obj.GetComponent<SapphirePhysics::RigidBody>())
    {
        RbComp->transform = obj.GetComponent<Transform>().get();
        RbComp->Init();
        if(RbComp->Static.Get() == true){
            RbComp->body->SetType(b2_staticBody);
        }else{
            RbComp->body->SetType(b2_dynamicBody);
        }
        if(obj.GetComponent<Renderer>() != nullptr)
            RbComp->ShapeType = static_cast<int>(obj.GetComponent<Renderer>()->ShapeType);
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
    b2Body* body = SapphirePhysics::RigidBody::world.GetBodyList();
    while (body) {
        b2Body* nextBody = body->GetNext(); // Get the next body before destroying the current one
        SapphirePhysics::RigidBody::world.DestroyBody(body); // Destroy the current body
        body = nextBody; // Move to the next body
    }
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
        for (auto &&comp : NewObj->GetComponents())
        {
            comp->Parent = NewObj;
        }
        
        //If the NewObj that was just loaded has any children, set their parent to be the new object.
        for (auto &child : children)
        {
            child->Parent = NewObj; 
        }
    }
    std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT, Engine::GetCameraObject()->GetRef());
    rend->Wireframe = true;
    rend->transform = Engine::GetCameraObject()->GetComponent<Transform>();
    Renderer::Gizmos.push_back(rend);
}