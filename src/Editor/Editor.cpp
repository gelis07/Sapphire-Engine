#include "Editor.h"
#include "DebugDraw.h"
#include "Engine/Scripting/LuaUtilities.h"
SapphireEngine::String Editor::ThemeName("ThemeName", Editor::UserPreferences);
SapphireEngine::Float Editor::AASamples("Anti aliasing samples", Engine::SettingsVariables);



void window_focus_callback(GLFWwindow* window, int focused)
{
    GamePreview* gp = static_cast<GamePreview*>(glfwGetWindowUserPointer(window));
    if (focused && !gp->GameRunning)
    {
        for (auto&& object : Engine::GetActiveScene().Objects)
        {
            for(auto&& component : object.GetComponents())
            {
                lua_State* L = component->GetState();
                if(L == nullptr) continue;
                component->GetLuaVariables();
            }
        }
    }
}

Editor::Editor(const std::string &mainPath) : Application(glm::vec2(960,540),true,mainPath),gp(this) , vp(), hierachy(&vp) 
{
    MainPath = AppMainPath;
    CurrentPath = AppMainPath;
    std::ifstream stream(MainPath + "/../ProjectSettings.json");
    nlohmann::ordered_json Data;
    stream >> Data;
    for (auto &&setting : Data.items())
    {
        Engine::SettingsVariables[setting.key()]->Load(setting.value());
    }
    stream.close();
    {
        std::ifstream stream("Assets/Preferences.json");
        nlohmann::ordered_json Data;
        try{
            stream >> Data;
        }
        catch (...){
            std::ofstream stream("Assets/Preferences.json");
            ThemeName.Get() = std::string("purple"); 
        }
        for (auto &&setting : Data.items())
        {
            UserPreferences[setting.key()]->Load(setting.value());
        }
        Load(ThemeName.Get());
    }
    SapphireEngine::Init();

    AASamples.Get() = 4;
    AASamples.Max = 24;
    AASamples.Min = 1;
    vp.Init();
}

void Editor::OnResize(GLFWwindow *window, int width, int height)
{
}
void Editor::OnUpdate(const float DeltaTime)
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
    Toolbar();
    SapphireEngine::ClearData();
    for (auto &&window : Window::Windows)
    {
        if(window->GetState())
            window->Update();
    }
    
    // AnimationTimeline();

    if(GetInput(GLFW_KEY_LEFT_CONTROL) && GetInputDown(GLFW_KEY_D)){
        DuplicateObj();
    }
    if(GetInput(GLFW_KEY_LEFT_CONTROL) && GetInput(GLFW_KEY_S)){
        if(Engine::GetActiveScene().SceneFile != ""){
            Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);
        }else{
            ImGui::OpenPopup("FileSaveMenu");
        }
    }
    this->DeltaTime = DeltaTime;
    if(vp.SelectedObj != null_ref){
        if(SelectedObjChildID == -1){
            vp.SelectedObj->Inspect();
        }else{
            if(SelectedObjChildID >= Engine::GetActiveScene().Objects[SelectedObjID].Children.size()){
                SelectedObjChildID = -1;
                vp.SelectedObj->Inspect();
            }else{
                vp.SelectedObj->Inspect();
            }
        }
    }
}

void Editor::DuplicateObj()
{
    Object *selectedObj = &Engine::GetActiveScene().Objects[SelectedObjID];
    Object NewObj(selectedObj->Name + " Copy");
    if (selectedObj->GetComponent<Renderer>() != nullptr)
    {
        NewObj.Components.push_back(std::shared_ptr<Renderer>(new Renderer(*selectedObj->GetComponent<Renderer>())));
        NewObj.GetComponent<Renderer>() = NewObj.GetComponent<Renderer>();
    }
    if (selectedObj->GetComponent<SapphirePhysics::RigidBody>() != nullptr)
    {
        NewObj.Components.push_back(std::shared_ptr<SapphirePhysics::RigidBody>(new SapphirePhysics::RigidBody(*selectedObj->GetComponent<SapphirePhysics::RigidBody>())));
        NewObj.GetComponent<SapphirePhysics::RigidBody>() = NewObj.GetComponent<SapphirePhysics::RigidBody>();
    }
    if (selectedObj->GetComponent<Transform>() != nullptr)
    {
        NewObj.Components.push_back(std::shared_ptr<Transform>(new Transform(*selectedObj->GetComponent<Transform>())));
        NewObj.GetComponent<Transform>() = NewObj.GetComponent<Transform>();
    }
    NewObj.GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(NewObj.GetComponent<Renderer>()->ShapeType);
    for (auto &&component : selectedObj->Components)
    {
        if (component->GetState() == nullptr)
            continue;
        NewObj.Components.push_back(std::shared_ptr<Component>(new Component(*component)));
    }
    NewObj.GetComponent<Transform>()->Move(glm::vec3(20, 20, 0));
    Engine::GetActiveScene().Add(std::move(NewObj));
    SelectedObjID = Engine::GetActiveScene().Objects.size() - 1;
}

void Editor::OnStart()
{
    glfwSetWindowUserPointer(window, &gp);
    glfwSetWindowFocusCallback(window, window_focus_callback);
}

void Editor::OnExit()
{
    {
        nlohmann::ordered_json ProjectSettingsJSON;
        std::ofstream stream(MainPath + "/../ProjectSettings.json");
        for (auto &&setting : Engine::SettingsVariables)
        {
            setting.second->Save(ProjectSettingsJSON);
        }
        stream << ProjectSettingsJSON.dump(2);
        stream.close();
    }
    {
        nlohmann::ordered_json UserPrefrencesJSON;
        std::ofstream stream("Assets/Preferences.json");
        for (auto &&setting : UserPreferences)
        {
            setting.second->Save(UserPrefrencesJSON);
        }
        stream << UserPrefrencesJSON.dump(2);
        stream.close();
    }
}
//Thanks The Cherno for the amazing tutorial! https://www.youtube.com/watch?v=Pegb5CZuibU
bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
{
    // From glm::decompose in matrix_decompose.inl
    using namespace glm;
    using T = float;

    mat4 LocalMatrix(transform);

    // Normalize the matrix.
    if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
        return false;

    // First, isolate perspective.  This is the messiest.
    if (
        epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
        epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
        epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
    {
        // Clear the perspective partition
        LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
        LocalMatrix[3][3] = static_cast<T>(1);
    }

    // Next take care of translation (easy).
    translation = vec3(LocalMatrix[3]);
    LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

    vec3 Row[3], Pdum3;

    // Now get scale and shear.
    for (length_t i = 0; i < 3; ++i)
        for (length_t j = 0; j < 3; ++j)
            Row[i][j] = LocalMatrix[i][j];

    // Compute X scale factor and normalize first row.
    scale.x = length(Row[0]);
    Row[0] = detail::scale(Row[0], static_cast<T>(1));
    scale.y = length(Row[1]);
    Row[1] = detail::scale(Row[1], static_cast<T>(1));
    scale.z = length(Row[2]);
    Row[2] = detail::scale(Row[2], static_cast<T>(1));

    rotation.y = asin(-Row[0][2]);
    if (cos(rotation.y) != 0) {
        rotation.x = atan2(Row[1][2], Row[2][2]);
        rotation.z = atan2(Row[0][1], Row[0][0]);
    }
    else {
        rotation.x = atan2(-Row[2][0], Row[1][1]);
        rotation.z = 0;
    }


    return true;
}

void Editor::OnWindowFocus(GLFWwindow *window, int focused)
{
    if (focused)
    {
        for (auto&& object : Engine::GetActiveScene().Objects)
        {
            for(auto&& component : object.GetComponents())
            {
                component->GetLuaVariables();
            }
        }
    }
}

const std::string &Editor::GetMainPath()
{
    return MainPath;
}
