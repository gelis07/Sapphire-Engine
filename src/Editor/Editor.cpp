#include "Editor.h"
#include "UI/FileExplorer/FileExplorer.h"
#include "DebugDraw.h"
#include "Engine/Scripting/LuaUtilities.h"
SapphireEngine::String Editor::ThemeName("ThemeName", Editor::UserPreferences);
SapphireEngine::Float Editor::AASamples("Anti aliasing samples", Engine::SettingsVariables);



void window_focus_callback(GLFWwindow* window, int focused)
{
    if (focused && !Editor::GameRunning)
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

Editor::Editor(const std::string &mainPath) : Application(glm::vec2(960,540),true,mainPath) , engine(), ViewCamera("Camera")
{
    engine.SetApp(this);
    PlayModeFBO.Init();
    ViewportFBO.Init();
    grid.Init();
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
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,0.5f,0));
    points.push_back(glm::vec3(-0.5f,0.5f,0));
    ViewCamera.Transform = std::make_shared<Transform>("Transform",std::move(points));
    ViewCamera.Transform->SetSize(glm::vec3(1 TOUNITS, 1 TOUNITS, 0.0f));

    std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT);
    rend->Wireframe = true;
    rend->transform = Engine::GetCameraObject()->GetComponent<Transform>();
    Renderer::Gizmos.push_back(rend);

    AASamples.Get() = 4;
    AASamples.Max = 24;
    AASamples.Min = 1;
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
    FileExplorer::Open(CurrentPath);
    if(SelectedObjID != -1){
        if(SelectedObjChildID == -1){
            Engine::GetActiveScene().Objects[SelectedObjID].Inspect();
        }else{
            if(SelectedObjChildID >= Engine::GetActiveScene().Objects[SelectedObjID].Children.size()){
                SelectedObjChildID = -1;
                Engine::GetActiveScene().Objects[SelectedObjID].Inspect();
            }else{
                Engine::GetActiveScene().Objects[SelectedObjID].Children[SelectedObjChildID]->Inspect();
            }
        }
    }
    Engine::GetActiveScene().Hierechy(&Engine::GetActiveScene().Objects[SelectedObjID], SelectedObjID);
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
    // NewObj.GetComponent<Renderer>()->shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices);
    // NewObj.GetComponent<Renderer>()->shape->ShapeType = SapphireRenderer::RectangleT;
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
void Editor::Zooming(GLFWwindow* window, double xoffset, double yoffset){
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if(yoffset < 0.0f && camera->Zoom.Get() <= 0.1f){
        camera->Zoom.Get() = 0.1f;
        return;
    }
    if(yoffset > 0 || camera->Zoom.Get() > 0.5f)
        camera->Zoom.Get() += yoffset/10;

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
    RenderFrameBufferViewport();
    bool retFlag;
    ViewportGUI(retFlag);
}
void Editor::ViewportGUI(bool &retFlag)
{
    retFlag = true;
    if (!(*Editor::GetWindowState("Viewport")))
        return;
    if (!ImGui::Begin("Viewport", Editor::GetWindowState("Viewport")))
    {
        // ImGui::End();
        // return;
    }

    WindowWidth = ImGui::GetContentRegionAvail().x;
    WindowHeight = ImGui::GetContentRegionAvail().y;
    MoveCamera(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
    ViewCamera.Transform->SetSize(glm::vec3(WindowWidth, WindowHeight, 0) TOUNITS);
    Engine::GetCameraObject()->GetComponent<Transform>()->SetSize(glm::vec3(WindowWidth, WindowHeight, 0) TOUNITS);
    glfwSetWindowUserPointer(window, &ViewCamera);
    glfwSetScrollCallback(window, ImGui::IsWindowHovered() ? Zooming : Default);

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID *>(ViewportFBO.RendFrameBuffer.GetTexture().GetID()),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + WindowWidth, pos.y + WindowHeight),
        ImVec2(0, 1),
        ImVec2(1, 0));
    // Thanks The Cherno for the amazing tutorial! https://www.youtube.com/watch?v=Pegb5CZuibU
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x + offset.x, ImGui::GetWindowPos().y + offset.y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    if (SelectedObjID != -1)
    {
        Gizmos();
    }

    ImGui::End();
    retFlag = false;
}
void Editor::RenderFrameBufferViewport()
{
    ViewportFBO.Bind();

    ViewportFBO.RescaleFrameBuffer(WindowWidth, WindowHeight);
    GLCall(glViewport(0, 0, WindowWidth, WindowHeight));
    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    grid.Render(ViewCamera.Transform->GetPosition() TOPIXELS, ViewCamera.Zoom.Get());
    Renderer::Render(&ViewCamera, Renderer::SceneRenderers);
    Renderer::Render(&ViewCamera, Renderer::Gizmos);
    SapphireEngine::DrawDebug(ViewCamera.Transform->GetModel());
    SapphireEngine::ClearData();
    ViewportFBO.Blit(WindowWidth, WindowHeight);
    ViewportFBO.Unbind();
}
void Editor::Gizmos()
{
    const glm::vec3 &Position = Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->GetPosition() TOPIXELS;
    const glm::vec3 &Rotation = Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->GetRotation();
    const glm::vec3 &Scale = Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->GetSize() TOPIXELS;
    glm::mat4 proj = glm::ortho(0.0f, ImGui::GetWindowSize().x / ViewCamera.Zoom.Get(), 0.0f, ImGui::GetWindowSize().y / ViewCamera.Zoom.Get(), -1.0f, 1.0f);
    // glm::mat4 proj = glm::ortho( -ImGui::GetWindowSize().x/2.0f / ViewCamera.Zoom, ImGui::GetWindowSize().x/2.0f / ViewCamera.Zoom, -ImGui::GetWindowSize().y / 2.0f / ViewCamera.Zoom, ImGui::GetWindowSize().y / 2.0f / ViewCamera.Zoom, -1.0f, 1.0f);

    glm::mat4 model = Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->GetModel();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_Operation = ImGuizmo::OPERATION::TRANSLATE;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_Operation = ImGuizmo::OPERATION::ROTATE;
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_Operation = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::Manipulate(glm::value_ptr(ViewCamera.GetView()), glm::value_ptr(proj), m_Operation, ImGuizmo::WORLD, glm::value_ptr(model));
    if (ImGuizmo::IsUsing())
    {
        glm::vec3 translation, rotation, scale;
        DecomposeTransform(model, translation, rotation, scale);

        if (m_Operation == ImGuizmo::OPERATION::TRANSLATE)
            Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->SetPosition(translation TOUNITS);
        else if (m_Operation == ImGuizmo::OPERATION::SCALE)
            Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->SetSize(scale TOUNITS);
        else if (m_Operation == ImGuizmo::OPERATION::ROTATE)
        {
            glm::vec3 deltaRotation = rotation - Rotation;
            Engine::GetActiveScene().Objects[SelectedObjID].GetComponent<Transform>()->Rotate(deltaRotation.z);
        }
    }
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
                LastPos = CursorPosToWind - glm::vec2(ViewCamera.Transform->GetPosition() TOPIXELS);
                FirstTimeClicking = false;
            }
            ViewCamera.Transform->SetPosition(glm::vec3(CursorPosToWind.x - LastPos.x, CursorPosToWind.y - LastPos.y, 0) TOUNITS);
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
bool NextFrame = false;
void Editor::RenderPlayMode()
{
    
    PlayModeFBO.Bind();

    PlayModeFBO.RescaleFrameBuffer(WindowWidth, WindowHeight);
    GLCall(glViewport(0, 0, WindowWidth, WindowHeight));
    const glm::vec4& ClearColor = Engine::GetCameraObject()->GetComponent<Camera>()->BgColor.Get();
    GLCall(glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(!GameRunning && !Start){
        OnGameRestart();
    } 
    if(!GameRunning || (Paused && !NextFrame)){
        //If the game is not running. Just render everything.
        Renderer::Render(Engine::GetCameraObject()->GetComponent<Camera>().get(), Renderer::SceneRenderers);
    }
    if((GameRunning && !Paused) || NextFrame){
        //The game is running so the engine should start running.
        SapphireEngine::ClearData();
        float time = glfwGetTime();
        engine.Run();
        TimeStep = (glfwGetTime() - time)*1000.0f;
        SapphireEngine::FrameCount++;
        SapphireEngine::FrameRate = (SapphireEngine::FrameRate + (glfwGetTime() - time))/SapphireEngine::FrameCount;
        NextFrame = false;
    }

    //Changing the start bool to false here so all the start functions get executed
    if(GameRunning) Start = false;
    PlayModeFBO.Blit(WindowWidth, WindowHeight);
    PlayModeFBO.Unbind();

    PlayModeGUI();
}

void Editor::PlayModeGUI()
{
    if ((*Editor::GetWindowState("Play")))
    {
        if (!ImGui::Begin("Play", Editor::GetWindowState("Play")))
        {
            // ImGui::End();
            // return;
        }
        // Engine::GetCameraObject()->GetComponent<Transform>()->SetSize(glm::vec3(WindowWidth, WindowHeight, 0) TOUNITS);
        WindowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        WindowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID *>(PlayModeFBO.RendFrameBuffer.GetTexture().GetID()),
            ImVec2(pos.x, pos.y),
            ImVec2(pos.x + WindowWidth, pos.y + WindowHeight),
            ImVec2(0, 1),
            ImVec2(1, 0));

        std::string Label;
        if (!GameRunning)
            Label = "Play";
        else
            Label = "Stop";
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
        // Set the ImGui Button to play the game
        if (ImGui::Button(Label.c_str()))
        {
            if (Engine::GetActiveScene().SceneFile == "")
            {
                ImGui::OpenPopup("Save Menu");
            }
            else
            {
                if (!GameRunning)
                    Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);

                if (CheckForErrors())
                {
                    GameRunning = !GameRunning;
                }
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
            }
        }
        ImGui::SetCursorPos(ImVec2(20, 20));
        std::stringstream ss;
        ss << (TimeStep) << ", Objects: " << Engine::GetActiveScene().Objects.size();
        ImGui::Text(ss.str().c_str());
        if (GameRunning)
        {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - 100, 20));
            // Set the ImGui Button to play the game
            if (ImGui::Button("Pause"))
            {
                Paused = !Paused;
            }
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - 200, 20));
            // Skip a frame while paused for debugging.
            if (ImGui::Button("Next Frame") && Paused)
            {
                NextFrame = true;
            }
        }
        if (ImGui::BeginPopup("Save Menu"))
        {
            ImGui::InputText("Scene Name", &SceneFileName, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::MenuItem("Save") || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            {
                if (CheckForErrors())
                {
                    Engine::GetActiveScene().Save(std::string(SceneFileName) + ".scene");
                    GameRunning = !GameRunning;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }
}

void Editor::OnGameRestart()
{
    // This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
    Engine::GetActiveScene().Load(Engine::GetActiveScene().SceneFile);
    Start = true;
    Paused = false;
    for (auto const &stat : SapphireEngine::stats)
    {
        std::cout << stat.first << ": " << stat.second << '\n';
    }
    std::cout << "Avg frame delta time: " << SapphireEngine::FrameRate << ", Frame count: " << SapphireEngine::FrameCount << '\n';
    SapphireEngine::FrameCount = 0;
    Engine::GameTime = 0.0f;
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