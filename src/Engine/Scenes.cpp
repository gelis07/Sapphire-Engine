#include "Scenes.h"
#include "json.hpp"
#include <vector>
#include "Engine.h"
#include "Editor.h"
void Scene::Save(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ofstream stream(Engine::GetMainPath() + FilePath, std::ofstream::trunc);
    nlohmann::json Data;
    unsigned int i = 0;
    nlohmann::json JsonObj;
    for (auto &obj : Objects)
    {
        nlohmann::json JsonComponents;
        JsonObj["Name"] = obj.Name;
        for (auto &component : obj.GetComponents())
        {
            nlohmann::json JsonComp;
            JsonComp["path"] = component->GetFile();
            JsonComp["Variables"] = component->Save(); // component->Save returns a json with all the variables
            JsonComponents[component->Name] = JsonComp;
        }
        JsonObj["Components"] = JsonComponents;
        std::stringstream ss;
        ss << "Object: " << i;
        if(obj.GetComponent<Renderer>() != nullptr)
            JsonObj["shape"] = obj.GetComponent<Renderer>()->shape->ShapeType;
        Data[ss.str().c_str()] = JsonObj;
        i++;
    }
    stream << Data.dump(2);
    stream.close();
}

void Scene::Load(const std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ifstream stream(Engine::GetMainPath() + FilePath);
    nlohmann::json Data;
    stream >> Data;
    stream.close();
    Objects.clear();
    for (size_t i = 0; i < Data.size(); i++)
    {
        std::stringstream ss;
        ss << "Object: " << i;
        nlohmann::json JsonObj = Data[ss.str().c_str()];

        std::shared_ptr<SapphireRenderer::Shape> shape = nullptr;
        Object obj(JsonObj["Name"]);
        nlohmann::json &JsonComp = JsonObj["Components"];
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
        

        Objects.push_back(obj);
    }
}

void Scene::Hierechy(Object *SelectedObj, int &SelectedObjID)
{
    if (!(*Editor::GetWindowState("Hierachy")))
        return;
    ImGui::Begin("Hierachy", Editor::GetWindowState("Hierachy"));
    for (size_t i = 0; i < Objects.size(); i++)
    {
        std::string Name = "";
        Name = Objects[i].Name.c_str();
        if (Name.empty())
        {
            Name = "##";
        }
        if (ImGui::Selectable((Name + "##" + std::to_string(Objects[i].id)).c_str(), &Objects[i] == SelectedObj))
        {
            SelectedObjID = i;
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