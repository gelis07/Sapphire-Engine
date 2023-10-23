#include "PlayMode.h"
#include "Engine/Engine.h"
#include "RunTime/RunTime.h"
void PlayMode::Init(Scene* activeScene)
{
    m_Window = glfwGetCurrentContext();
    m_ActiveScene = activeScene;
    FrameBuffer.Init();
    Engine::Get().GetWindows().InitWindow("Play");

    CameraObject = std::make_shared<Object>("MainCamera");

    CameraObject->AddComponent<Transform>(new Transform("", "Transform", 1, CameraObject.get(),false));
    CameraObject->AddComponent<Camera>(new Camera("", "Camera", 2,  CameraObject.get(),false));
    CameraObject->AddComponent<Renderer>(new Renderer("", "Renderer", 3, CameraObject.get(), false));

    CameraObject->GetTransform() = CameraObject->GetComponent<Transform>();
    CameraObject->GetRenderer() = CameraObject->GetComponent<Renderer>();
    
    CameraObject->GetRenderer()->shape = std::make_shared<Shapes::CameraGizmo>(Shapes::BasicShader);
    CameraObject->GetRenderer()->shape->Wireframe() = true;
    activeScene->Objects.push_back(CameraObject);
}

bool CheckForErrors()
{
    for (auto&& object : Engine::Get().GetActiveScene()->Objects)
    {
        //Should limit this only to the 
        for(auto&& component : object->GetComponents())
        {
            if(component->GetState() == nullptr) 
                continue;
            if(!ScriptingEngine::CheckLua(component->GetState(), luaL_dofile(component->GetState(), (Engine::Get().GetMainPath() + component->GetFile()).c_str())))
            {
                return false;
            }
        }
    }
    return true;
}

void PlayMode::Render(std::string& MainPath)
{
    if((*Engine::Get().GetWindows().GetWindowState("Play"))){
        if(!ImGui::Begin("Play", Engine::Get().GetWindows().GetWindowState("Play"))){
            // ImGui::End();
            // return;
        }

        // glfwGetWindowSize(glfwGetCurrentContext(), &m_WindowWidth, &m_WindowHeight);
        m_WindowWidth = ImGui::GetContentRegionAvail().x;
        m_WindowHeight = ImGui::GetContentRegionAvail().y;
        CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(m_WindowWidth, m_WindowHeight, 0);
        
        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID*>(FrameBuffer.GetID()), 
            ImVec2(pos.x, pos.y), 
            ImVec2(pos.x + m_WindowWidth, pos.y + m_WindowHeight), 
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
            if(m_ActiveScene->SceneFile == ""){
                ImGui::OpenPopup("Save Menu");
            }else{
                if(!GameRunning) m_ActiveScene->Save(m_ActiveScene->SceneFile);

                if(CheckForErrors())
                    GameRunning = !GameRunning;
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
            }
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
            ImGui::InputText("Scene Name", &m_SceneFileName, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::MenuItem("Save") || glfwGetKey(m_Window, GLFW_KEY_ENTER) == GLFW_PRESS)
            {
                if(CheckForErrors()){
                    m_ActiveScene->Save(std::string(m_SceneFileName) + ".scene");
                    GameRunning = !GameRunning;
                }
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
                    
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }
    

    FrameBuffer.Bind();
    FrameBuffer.RescaleFrameBuffer(m_WindowWidth, m_WindowHeight);
    GLCall(glViewport(0, 0, m_WindowWidth, m_WindowHeight));

    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(!GameRunning && !m_Start){
        //This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
        //The start functions get called
        m_ActiveScene->Load(m_ActiveScene->SceneFile);
        m_Start = true;
        RunTime::Time = 0.0f;
        Paused = false;
    } 
    if(!GameRunning){
        for (size_t i = 0; i < m_ActiveScene->Objects.size(); i++)
        {
            RunTime::Render(m_ActiveScene->Objects[i], CameraObject);
        }
    }
    if(GameRunning){
        RunTime::Run(m_ActiveScene, CameraObject, Engine::Get().GetDeltaTime());
    }

    //Changing the start bool to false here so all the start functions get executed
    if(GameRunning) m_Start = false;
    FrameBuffer.Unbind();
}