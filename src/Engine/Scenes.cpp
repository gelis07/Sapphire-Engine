#include "Scenes.h"
#include "UI/Windows.h"
#include "json.hpp"
#include <vector>
#include "Engine.h"

void Scene::Save(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ofstream stream(Engine::Get().GetMainPath() + FilePath, std::ofstream::trunc);
    nlohmann::json Data;
    unsigned int i = 0;
    nlohmann::json JsonObj;
    for(const auto& obj : Objects){
        nlohmann::json JsonComponents;
        JsonObj["Name"] = obj->Name;
        for (auto &component : obj->GetComponents())
        {
            nlohmann::json JsonComp;
            JsonComp["path"] = component->GetFile();
            JsonComp["Variables"] = component->Save(); // component->Save returns a json with all the variables
            JsonComponents[component->Name] = JsonComp;
        }
        JsonObj["Components"] = JsonComponents;
        std::stringstream ss;
        ss << "Object: " << i;
        JsonObj["shape"] = obj->GetComponent<Renderer>()->shape->ShapeType;
        Data[ss.str().c_str()] = JsonObj; 
        i++;
    }
    stream << Data.dump(2);
    stream.close();
}


void Scene::Load(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ifstream stream(Engine::Get().GetMainPath() + FilePath);
    nlohmann::json Data;
    stream >> Data;
    stream.close();
    for (auto &&object : Objects)
    {
        object->GetComponents().clear();
    }
    Objects.clear();
    for (size_t i = 0; i < Data.size(); i++)
    {
        std::stringstream ss;
        ss << "Object: " << i;
        nlohmann::json JsonObj = Data[ss.str().c_str()];

        std::shared_ptr<Shapes::Shape> shape;
        std::shared_ptr<Object> obj = std::make_shared<Object>(JsonObj["Name"]);
        switch (JsonObj["shape"].get<int>())
        {
            case Shapes::CircleT:
                shape = std::make_shared<Shapes::Circle>(Shapes::CircleShader);
                break;
            case Shapes::RectangleT:
                shape = std::make_shared<Shapes::Rectangle>(Shapes::BasicShader);
                break;
            default:
                shape = nullptr;
        }
        nlohmann::json& JsonComp = JsonObj["Components"];
        for (auto& element : JsonObj["Components"].items()) {
            //! Got to find a better way to handle this!
            if(element.key() == "Renderer")
            {
                Renderer* comp = new Renderer(element.value()["path"], element.key(), obj->GetComponents().size(), obj.get(), element.value()["path"] != "");
                obj->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Renderer>(dynamic_cast<Renderer*>(comp))));
                obj->GetComponents().back()->Load(element.value()["Variables"]);
            }
            else if(element.key() == "Transform")
            {
                Transform* comp = new Transform(element.value()["path"], element.key(), obj->GetComponents().size(), obj.get(),element.value()["path"] != "");
                obj->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Transform>(dynamic_cast<Transform*>(comp))));
                obj->GetComponents().back()->Load(element.value()["Variables"]);
            }else if(element.key() == "Camera") {
                Camera* comp = new Camera(element.value()["path"], element.key(), obj->GetComponents().size(), obj.get(),element.value()["path"] != "");
                obj->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<Camera>(dynamic_cast<Camera*>(comp))));
                obj->GetComponents().back()->Load(element.value()["Variables"]);
                shape = std::make_shared<Shapes::CameraGizmo>(Shapes::BasicShader);
                shape->Wireframe() = true;
                Engine::Get().GetPlay().CameraObject = obj;
            }
            else if(element.key() == "Rigidbody") {
                RigidBody* comp = new RigidBody(element.value()["path"], element.key(), obj->GetComponents().size(), obj.get(),element.value()["path"] != "");
                obj->GetComponents().push_back(std::static_pointer_cast<Component>(std::shared_ptr<RigidBody>(dynamic_cast<RigidBody* >(comp))));
                obj->GetComponents().back()->Load(element.value()["Variables"]);
            }
            else
            {
                Component* comp = new Component(element.value()["path"], element.key(), obj->GetComponents().size(), obj.get(), element.value()["path"] != "");
                comp->Load(element.value()["Variables"]);
                obj->AddComponent<Component>(comp);
            }
        }
        if(obj == Engine::Get().GetPlay().CameraObject)
            obj->GetComponent<Transform>()->Size.value<glm::vec3>() = glm::vec3(Engine::Get().GetViewport().GetWindowSize().x, Engine::Get().GetViewport().GetWindowSize().y, 0);

        obj->GetTransform() = obj->GetComponent<Transform>();
        obj->GetRenderer() = obj->GetComponent<Renderer>();
        obj->GetComponent<Renderer>()->shape = shape;
        Objects.push_back(obj);
    }
    Engine::Get().GetViewport().SelectedObj = nullptr;
}

void Scene::Hierechy(std::shared_ptr<Object> &SelectedObj)
{
    if(!(*Engine::Get().GetWindows().GetWindowState("Hierachy"))) return;
    ImGui::Begin("Hierachy", Engine::Get().GetWindows().GetWindowState("Hierachy"));
    for (size_t i = 0; i < Objects.size(); i++)
    {
        std::string Name = "";
        Name = Objects[i]->Name.c_str();
        if (Name.empty())
        {
            Name = "##";
        }
        if(ImGui::Selectable((Name + "##" + std::to_string(Objects[i]->id)).c_str(), Objects[i] == SelectedObj)){
            SelectedObj = Objects[i];
        }
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context Menu");
    }
    CreateMenu(SelectedObj);
    ImGui::End();
}

void Scene::CreateMenu(std::shared_ptr<Object> &SelectedObj){
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
            std::shared_ptr<Object> Obj = Object::CreateObject(ss.str());
            Obj->GetComponent<Renderer>()->shape = std::make_shared<Shapes::Circle>(Shapes::CircleShader);
        }
        if (ImGui::MenuItem("Create Rectangle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            std::shared_ptr<Object> Obj = Object::CreateObject(ss.str());
            Obj->GetComponent<Renderer>()->shape = std::make_shared<Shapes::Rectangle>(Shapes::BasicShader);
        }
        ImGui::Separator();
        // if(ImGui::MenuItem("Duplicate")){
        //     DuplicateObject(SelectedObj, Objects);
        // }
        // if(ImGui::MenuItem("Delete")){
        //     DeleteObject(SelectedObj->id, Objects, SelectedObj);
        // }
        ImGui::EndPopup();
    }
}