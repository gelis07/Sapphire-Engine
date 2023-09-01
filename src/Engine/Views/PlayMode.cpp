#include "PlayMode.h"
#include "Engine/Engine.h"
#include "RunTime/RunTime.h"
void PlayMode::Init(Scene* activeScene)
{
    m_Window = glfwGetCurrentContext();
    m_ActiveScene = activeScene;
    m_Texture = CreateViewportTexture();
    m_FBO = CreateFBO(m_Texture);
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

void PlayMode::RescaleFrameBuffer(float width, float height)
{
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
        ImGui::Begin("Play", Engine::Get().GetWindows().GetWindowState("Play"));

        // glfwGetWindowSize(glfwGetCurrentContext(), &m_WindowWidth, &m_WindowHeight);
        m_WindowWidth = ImGui::GetContentRegionAvail().x;
        m_WindowHeight = ImGui::GetContentRegionAvail().y;
        CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(m_WindowWidth, m_WindowHeight, 0);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID*>(m_Texture), 
            ImVec2(pos.x, pos.y), 
            ImVec2(pos.x + m_WindowWidth, pos.y + m_WindowHeight), 
            ImVec2(0, 1), 
            ImVec2(1, 0)
        );

        std::string Label;
        if (Paused){ Label = "Play"; }
        else{ Label = "Pause"; }
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
        // Set the ImGui Button to play the game
        if (ImGui::Button(Label.c_str()))
        {
            if(m_ActiveScene->SceneFile == ""){
                ImGui::OpenPopup("Save Menu");
            }else{
                if(Paused) m_ActiveScene->Save(m_ActiveScene->SceneFile);

                if(CheckForErrors())
                    Paused = !Paused;
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
            }
        }
        if (ImGui::BeginPopup("Save Menu"))
        {
            ImGui::InputText("Scene Name", &m_SceneFileName, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::MenuItem("Save") || glfwGetKey(m_Window, GLFW_KEY_ENTER) == GLFW_PRESS)
            {
                if(CheckForErrors()){
                    m_ActiveScene->Save(std::string(m_SceneFileName) + ".scene");
                    Paused = !Paused;
                }
                else
                    SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
                    
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }
    

    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
    RescaleFrameBuffer(m_WindowWidth, m_WindowHeight);
    GLCall(glViewport(0, 0, m_WindowWidth, m_WindowHeight));

    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(Paused && !m_Start){
        //This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
        //The start functions get called
        m_ActiveScene->Load(m_ActiveScene->SceneFile);
        m_Start = true;
    } 
    if(Paused){
        for (size_t i = 0; i < m_ActiveScene->Objects.size(); i++)
        {
            RunTime::Render(m_ActiveScene->Objects[i], CameraObject);
        }
    }
    if(!Paused){
        RunTime::Run(m_ActiveScene, CameraObject, Engine::Get().GetDeltaTime());
    }

    //Changing the start bool to false here so all the start functions get executed
    if(!Paused) m_Start = false;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}