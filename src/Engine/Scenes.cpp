#include "Scenes.h"
#include "UI/Windows.h"
#include "json.hpp"
#include <vector>



//* Not the best name but it just gets the stored value of the variant and stores it in a json accordingly
void SetJSONVariable(Variable &var, const std::string name,nlohmann::json &Variables){
    if(std::holds_alternative<float>(var.Contents)){
        Variables[name] = {var.Type, std::get<float>(var.Contents)};
    }
    else if(std::holds_alternative<std::string>(var.Contents)){
        Variables[name] = {var.Type, std::get<std::string>(var.Contents)};
    }
    else if(std::holds_alternative<bool>(var.Contents)){
        Variables[name] = {var.Type, std::get<bool>(var.Contents)};
    }
    else if(std::holds_alternative<std::vector<LuaTableIt>>(var.Contents)){
        nlohmann::json Table;
        for(auto const& TableVariable : std::get<std::vector<LuaTableIt>>(var.Contents)){
            std::string VarName;
            if(std::holds_alternative<int>(TableVariable.Key)){
                VarName = std::to_string(std::get<int>(TableVariable.Key));
            }else if(std::holds_alternative<std::string>(TableVariable.Key)){
                VarName = std::get<std::string>(TableVariable.Key);
            }
            if(TableVariable.Type == LUA_TNUMBER){
                Table[VarName] = {TableVariable.Type, std::get<float>(TableVariable.Contents)};
            }
            else if(TableVariable.Type == LUA_TSTRING){
                Table[VarName] = {TableVariable.Type, std::get<std::string>(TableVariable.Contents)};
            }
            else if(TableVariable.Type == LUA_TBOOLEAN){
                Table[VarName] = {TableVariable.Type, std::get<bool>(TableVariable.Contents)};
            }
        }
        Variables[name] = {LUA_TTABLE, std::get<std::string>(var.Contents)};
    }else if(std::holds_alternative<glm::vec2>(var.Contents)){
        nlohmann::json vector;
        glm::vec2 VariableVector = std::get<glm::vec2>(var.Contents);
        vector["x"] = VariableVector.x;
        vector["y"] = VariableVector.y;
        Variables[name] = {LUA_TTABLE, vector};
    }else if(std::holds_alternative<glm::vec3>(var.Contents)){
        nlohmann::json vector;
        glm::vec3 VariableVector = std::get<glm::vec3>(var.Contents);
        vector["x"] = VariableVector.x;
        vector["y"] = VariableVector.y;
        vector["z"] = VariableVector.z;
        Variables[name] = {LUA_TTABLE, vector};
    }else if(std::holds_alternative<glm::vec4>(var.Contents)){
        nlohmann::json vector;
        glm::vec4 VariableVector = std::get<glm::vec4>(var.Contents);
        vector["x"] = VariableVector.x;
        vector["y"] = VariableVector.y;
        vector["z"] = VariableVector.z;
        vector["w"] = VariableVector.w;
        Variables[name] = {LUA_TTABLE, vector};
    }
}
    



void Scene::Save(std::string FilePath)
{
    this->SceneFile = FilePath;
    std::ofstream stream(Windows::MainPath + FilePath, std::ofstream::trunc);
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
            nlohmann::json Variables;
            for (auto &variable : component->Variables)
            {
                SetJSONVariable(variable.second, variable.first, Variables);
            }
            JsonComp["Variables"] = Variables;
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

void Scene::Load(std::string FilePath, GLFWwindow* window)
{
    this->SceneFile = FilePath;
    std::ifstream stream(Windows::MainPath + FilePath);
    nlohmann::json Data;
    stream >> Data;
    stream.close();
    Objects.clear();
    for (size_t i = 0; i < Data.size(); i++)
    {
        std::stringstream ss;
        ss << "Object: " << i;
        nlohmann::json JsonObj = Data[ss.str().c_str()];

        std::shared_ptr<Shapes::Shape> shape;
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
        std::shared_ptr<Object> obj = std::make_shared<Object>(std::move(JsonObj["Name"]));
        for (auto& element : JsonObj["Components"].items()) {
            Component* comp = new Component(element.value()["path"], element.key(), obj->GetComponents().size(), true);
            comp->Load(element.value()["Variables"]);
            //! Got to find a better way to handle this!
            if(comp->Name == "Renderer")
            {
                obj->AddComponent<Renderer>((Renderer*)comp);
            }
            else if(comp->Name == "Transform")
            {
                obj->AddComponent<Transform>((Transform*)comp);
            }else
            {
                obj->AddComponent<Component>(comp);
            }
        }

        int test = NULL;

        obj->GetComponent<Renderer>()->shape = shape;
        Objects.push_back(obj);
    }
}

void Scene::Hierechy(std::shared_ptr<Object> &SelectedObj)
{
    ImGui::Begin("Hierachy");
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
            Object::CreateObject(Objects, ss.str(), nullptr);
        }
        if (ImGui::MenuItem("Create Circle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object::CreateObject(Objects, ss.str(), std::make_shared<Shapes::Circle>(Shapes::CircleShader));
        }
        if (ImGui::MenuItem("Create Rectangle"))
        {
            std::stringstream ss;
            ss << "Object: " << Objects.size();
            Object::CreateObject(Objects, ss.str(), std::make_shared<Shapes::Rectangle>(Shapes::BasicShader));
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