#include "Editor.h"
#include "UI/FileExplorer/FileExplorer.h"
SapphireEngine::String Editor::ThemeName("ThemeName", Editor::UserPreferences);

Editor::Editor(const std::string &mainPath) : Application(glm::vec2(960,540),true,mainPath) , engine()
{
    engine.SetApp(this);
    PlayModeFBO.Init();
    ViewportFBO.Init();
    grid.Init();
    MainPath = AppMainPath;
    CurrentPath = AppMainPath;
    std::ifstream stream(MainPath + "/../ProjectSettings.json");
    nlohmann::json Data;
    stream >> Data;
    for (auto &&setting : Data.items())
    {
        Engine::SettingsVariables[setting.key()]->Load(setting.value());
    }
    stream.close();
    FileExplorer::Init();
    InitWindow("Inspector");
    InitWindow("Hierachy");
    InitWindow("Viewport");
    InitWindow("Logs");
    InitWindow("Play");
    InitWindow("FrameRate", false);
    InitWindow("Preferences", false);
    InitWindow("Performance", false);
    InitWindow("Settings", false);
    InitWindow("Project Settings", false);
    {
        std::ifstream stream("Assets/Preferences.json");
        nlohmann::json Data;
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
    RenderPlayMode();
    RenderViewport();
    AnimationTimeline();
    FrameRate();
    LogWindow();
    
    this->DeltaTime = DeltaTime;
    FileExplorer::Open(CurrentPath);
    if(SelectedObjID != -1) Engine::GetActiveScene().Objects[SelectedObjID].Inspect();
    Engine::GetActiveScene().Hierechy(&Engine::GetActiveScene().Objects[SelectedObjID], SelectedObjID);
}

void Editor::OnStart()
{
}

void Editor::OnExit()
{
    {
        nlohmann::json ProjectSettingsJSON;
        std::ofstream stream(MainPath + "/../ProjectSettings.json");
        for (auto &&setting : Engine::SettingsVariables)
        {
            setting.second->Save(ProjectSettingsJSON);
        }
        stream << ProjectSettingsJSON.dump(2);
        stream.close();
    }
    {
        nlohmann::json UserPrefrencesJSON;
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
void Editor::Zooming(GLFWwindow* window, double xoffset, double yoffset){
    ViewportCamera* camera = static_cast<ViewportCamera*>(glfwGetWindowUserPointer(window));
    if(yoffset < 0.0f && camera->Zoom <= 0.1f){
        camera->Zoom = 0.1f;
        return;
    }
    if(yoffset > 0 || camera->Zoom > 0.5f)
        camera->Zoom += yoffset/10;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
}

static void Default(GLFWwindow* window, double xoffset, double yoffset){
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
}
constexpr glm::vec2 offset = glm::vec2(8, -6);
void Editor::RenderViewport()
{
    if(!(*Editor::GetWindowState("Viewport"))) return;
    if(!ImGui::Begin("Viewport", Editor::GetWindowState("Viewport"))){
        // ImGui::End();
        // return;
    }

    WindowWidth = ImGui::GetContentRegionAvail().x;
    WindowHeight = ImGui::GetContentRegionAvail().y;

    MoveCamera(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
    glfwSetWindowUserPointer(window, &ViewCamera);
    glfwSetScrollCallback(window, ImGui::IsWindowHovered() ? Zooming : Default);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID*>(ViewportFBO.GetTexture().GetID()), 
        ImVec2(pos.x, pos.y), 
        ImVec2(pos.x + WindowWidth, pos.y + WindowHeight), 
        ImVec2(0, 1), 
        ImVec2(1, 0)
    );

    // Thanks The Cherno for the amazing tutorial! https://www.youtube.com/watch?v=Pegb5CZuibU
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    
    ImGuizmo::SetRect(ImGui::GetWindowPos().x + offset.x, ImGui::GetWindowPos().y + offset.y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    
    glm::mat4 view = glm::translate(glm::mat4(1.0f), ViewCamera.position + glm::vec3(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y, 0) / 2.0f);
    if(SelectedObjID != -1){
        const glm::vec3& Position = Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->GetPosition();
        const glm::vec3& Rotation = Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->GetRotation();
        const glm::vec3& Scale = Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->GetSize();
        glm::mat4 proj = glm::ortho(0.0f, ImGui::GetWindowSize().x / ViewCamera.Zoom, 0.0f, ImGui::GetWindowSize().y / ViewCamera.Zoom, -1.0f, 1.0f);
        // glm::mat4 proj = glm::ortho( -ImGui::GetWindowSize().x/2.0f / ViewCamera.Zoom, ImGui::GetWindowSize().x/2.0f / ViewCamera.Zoom, -ImGui::GetWindowSize().y / 2.0f / ViewCamera.Zoom, ImGui::GetWindowSize().y / 2.0f / ViewCamera.Zoom, -1.0f, 1.0f);


        glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
        glm::mat4 Transform = glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);


        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::TRANSLATE;
        else if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::ROTATE;
        else if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::SCALE;

        ImGuizmo::Manipulate(&view[0][0], &proj[0][0], m_Operation, ImGuizmo::WORLD, &Transform[0][0]);
        if(ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            DecomposeTransform(Transform, translation, rotation, scale);

            if(m_Operation == ImGuizmo::OPERATION::TRANSLATE)
                Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->SetPosition(translation);
            else if(m_Operation == ImGuizmo::OPERATION::SCALE)
                Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->SetSize(scale);
            else if(m_Operation == ImGuizmo::OPERATION::ROTATE){
                glm::vec3 deltaRotation = rotation - Rotation;
                Engine::GetActiveScene().Objects[SelectedObjID].GetTransform()->Rotate(deltaRotation.z);
            }
        }
    }


    ImGui::End();


    ViewportFBO.Bind();

    ViewportFBO.RescaleFrameBuffer(WindowWidth, WindowHeight);
    GLCall(glViewport(0, 0, WindowWidth, WindowHeight));
    
    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    grid.Render(ViewCamera.position, ViewCamera.Zoom);
    for (size_t i = 0; i < Engine::GetActiveScene().Objects.size(); i++)
    {
        if(std::shared_ptr<Renderer> renderer = Engine::GetActiveScene().Objects[i].GetRenderer())
            renderer->Render(*(Engine::GetActiveScene().Objects[i].GetTransform()), view, &Engine::GetActiveScene().Objects[i] == &Engine::GetActiveScene().Objects[SelectedObjID], ViewCamera.position, ViewCamera.Zoom);
        else{
            //Check if it does indeed exist and is not set to the renderer variable on the object set it,
            if(Engine::GetActiveScene().Objects[i].GetRenderer() = Engine::GetActiveScene().Objects[i].GetComponent<Renderer>()) 
                Engine::GetActiveScene().Objects[i].GetRenderer()->Render(*(Engine::GetActiveScene().Objects[i].GetTransform()), view, &Engine::GetActiveScene().Objects[i] == &Engine::GetActiveScene().Objects[SelectedObjID], ViewCamera.position, ViewCamera.Zoom);
            else
                SapphireEngine::Log(Engine::GetActiveScene().Objects[i].Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
        }
        Engine::GetActiveScene().Objects[i].id = i;
    }

    ViewportFBO.Unbind();
}
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
void Editor::MoveCamera(glm::vec2 &&Size, glm::vec2 &&Position)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    glm::vec2 CursorPosToWind((CursorPos.x - Position.x), (Position.y - CursorPos.y));
    if(CursorPosToWind.x > 0 && CursorPosToWind.x < Size.x && -CursorPosToWind.y > 0 && -CursorPosToWind.y < Size.y)
    {
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(FirstTimeClicking){
                LastPos = CursorPosToWind - glm::vec2(ViewCamera.position);
                FirstTimeClicking = false;
            }
            ViewCamera.position = glm::vec3(CursorPosToWind.x - LastPos.x, CursorPosToWind.y - LastPos.y, 0);
        }
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && !FirstTimeClicking) FirstTimeClicking = true;
}
bool CheckForErrors()
{
    for (auto&& object : Engine::GetActiveScene().Objects)
    {
        //Should limit this only to the 
        for(auto&& component : object.GetComponents())
        {
            if(component->GetState() == nullptr) 
                continue;
            if(!ScriptingEngine::CheckLua(component->GetState(), luaL_dofile(component->GetState(), (Editor::GetMainPath() + component->GetFile()).c_str())))
            {
                return false;
            }
        }
    }
    return true;
}
float TimeStep = 0;
void Editor::RenderPlayMode()
{
    if((*Editor::GetWindowState("Play"))){
        if(!ImGui::Begin("Play", Editor::GetWindowState("Play"))){
            // ImGui::End();
            // return;
        }

        // glfwGetWindowSize(glfwGetCurrentContext(), &WindowWidth, &WindowHeight);
        // WindowWidth = ImGui::GetContentRegionAvail().x;
        // WindowHeight = ImGui::GetContentRegionAvail().y;
        Engine::GetCameraObject()->GetTransform()->SetSize(glm::vec3(WindowWidth, WindowHeight, 0));
        WindowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        WindowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID*>(PlayModeFBO.GetTexture().GetID()), 
            ImVec2(pos.x, pos.y), 
            ImVec2(pos.x + WindowWidth, pos.y + WindowHeight), 
            ImVec2(0, 1), 
            ImVec2(1, 0)
        );

        std::string Label;
        if (!GameRunning){ Label = "Play"; }
        else{ Label = "Stop"; }
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
        // Set the ImGui Button to play the game
        if (ImGui::Button(Label.c_str()))
        {
            if(Engine::GetActiveScene().SceneFile == ""){
                ImGui::OpenPopup("Save Menu");
            }else{
                if(!GameRunning) Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);

                if(CheckForErrors()){
                    GameRunning = !GameRunning;
                    loggingFile.open("hello.txt", std::ios::out);
                }
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
            }
        }
        ImGui::SetCursorPos(ImVec2(20, 20));
        std::stringstream ss;
        ss << (TimeStep) << ", Objects: " << Engine::GetActiveScene().Objects.size();
        ImGui::Text(ss.str().c_str());

        if(loggingFile.is_open()){
            loggingFile << ss.str() << '\n';
        }
        if(GameRunning)
        {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - 100, 20));
            // Set the ImGui Button to play the game
            if (ImGui::Button("Pause"))
            {
                Paused = !Paused;
            }
        }
        if (ImGui::BeginPopup("Save Menu"))
        {
            ImGui::InputText("Scene Name", &SceneFileName, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::MenuItem("Save") || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            {
                if(CheckForErrors()){
                    Engine::GetActiveScene().Save(std::string(SceneFileName) + ".scene");
                    GameRunning = !GameRunning;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }
    

    PlayModeFBO.Bind();

    PlayModeFBO.RescaleFrameBuffer(WindowWidth, WindowHeight);
    GLCall(glViewport(0, 0, WindowWidth, WindowHeight));

    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(!GameRunning && !Start){
        //This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
        //The start functions get called
        Engine::GetActiveScene().Load(Engine::GetActiveScene().SceneFile);
        Start = true;
        Paused = false;
        loggingFile.close();
    } 
    if(!GameRunning){
        for (size_t i = 0; i < Engine::GetActiveScene().Objects.size(); i++)
        {
            engine.Render(&Engine::GetActiveScene().Objects[i]);
        }
    }
    if(GameRunning && !Paused){
        float time = glfwGetTime();
        engine.Run();
        TimeStep = (glfwGetTime() - time)*1000.0f;
    }

    //Changing the start bool to false here so all the start functions get executed
    if(GameRunning) Start = false;
    PlayModeFBO.Unbind();
}

void Editor::OnWindowFocus(GLFWwindow *window, int focused)
{
}

const std::string &Editor::GetMainPath()
{
    return MainPath;
}

void Editor::InitWindow(std::string &&WindowName, bool state)
{
    WindowStates[WindowName] = state;
}

void Editor::SetWindowState(std::string &&WindowName, bool state)
{
    if(WindowStates.find(WindowName) != WindowStates.end()){
        WindowStates[WindowName] = state;
    }else{
        SapphireEngine::Log(WindowName + " window doesn't exist", SapphireEngine::Error);
    }
}

bool* Editor::GetWindowState(std::string &&WindowName)
{
    if(WindowStates.find(WindowName) != WindowStates.end()){
        return &WindowStates[WindowName];
    }else{
        SapphireEngine::Log(WindowName + " window doesn't exist", SapphireEngine::Error);
    }
    return nullptr;
}