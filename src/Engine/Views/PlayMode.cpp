#include "PlayMode.h"
#include "Engine/Engine.h"
void PlayMode::Init(Scene* activeScene)
{
    m_Window = glfwGetCurrentContext();
    m_ActiveScene = activeScene;
    m_Texture = CreateViewportTexture();
    m_FBO = CreateFBO(m_Texture);

    CameraObject = std::make_shared<Object>("MainCamera");

    CameraObject->AddComponent<Transform>(new Transform("", "Transform", 1, false));
    CameraObject->AddComponent<Camera>(new Camera("", "Camera", 2, false));
    CameraObject->AddComponent<Renderer>(new Renderer("", "Renderer", 3, false));

    CameraObject->GetTransform() = CameraObject->GetComponent<Transform>();
    CameraObject->GetRenderer() = CameraObject->GetComponent<Renderer>();
    
    CameraObject->GetRenderer()->shape = std::make_shared<Shapes::CameraGizmo>(Shapes::BasicShader, CameraObject);
    CameraObject->GetRenderer()->shape->Wireframe() = true;
    CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    activeScene->Objects.push_back(CameraObject);
}

void PlayMode::Render(std::string& MainPath) //Here we need the main path because we load the scene after the user hops out of play mode
{
    if(!ImGuiRender(m_Texture, MainPath)) return;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
    glm::vec4& Color = CameraObject->GetComponent<Camera>()->BgColor.value<glm::vec4>();
    GLCall(glClearColor(Color.r, Color.g, Color.b, Color.a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(Paused && !m_Start){
        //This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
        //The start functions get called
        m_ActiveScene->Load(m_ActiveScene->SceneFile);
        m_Start = true;
    } 
    try {
        for (size_t i = 0; i < m_ActiveScene->Objects.size(); i++)
        {
            if(std::shared_ptr<Renderer> renderer = m_ActiveScene->Objects[i]->GetRenderer()) {
                if(m_ActiveScene->Objects[i] != CameraObject)
                    renderer->Render(false, -CameraObject->GetTransform()->Position.value<glm::vec3>(),CameraObject->GetComponent<Camera>()->Zoom.value<float>(), false);
            }
            else
                if(m_ActiveScene->Objects[i]->GetRenderer() = m_ActiveScene->Objects[i]->GetComponent<Renderer>()) 
                    m_ActiveScene->Objects[i]->GetRenderer()->Render(false, -CameraObject->GetTransform()->Position.value<glm::vec3>(),CameraObject->GetComponent<Camera>()->Zoom.value<float>(), false);
                else
                    SapphireEngine::Log(m_ActiveScene->Objects[i]->Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
            if(!Paused){
                m_ActiveScene->Objects[i]->OnStart();
                m_ActiveScene->Objects[i]->OnUpdate();
                if(std::shared_ptr<RigidBody> rb = m_ActiveScene->Objects[i]->GetComponent<RigidBody>()) {
                    rb->CheckForCollisions(m_ActiveScene->Objects[i].get());
                    rb->Simulate(m_ActiveScene->Objects[i].get());
                }
            }
        }
        //Changing the start bool to false here so all the start functions get executed
        if(!Paused) m_Start = false;
    }
    catch (const char* msg) {
        Paused = true;
        SapphireEngine::Log(std::string(msg), SapphireEngine::Error);
    } 
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

char Name[256] = "";

bool PlayMode::ImGuiRender(unsigned int texture, std::string& MainPath)
{
    bool IsActive = ImGui::Begin("Play");

    // Size the image of the window to a steady aspect ratio
    ImTextureID textureID = (void *)(intptr_t)texture;
    ImVec2 uv0 = ImVec2(0, 1);
    ImVec2 uv1 = ImVec2(1, 0);
    ImGui::Image(textureID, ScaleWindow(), uv0, uv1);

    std::string Label;
    if (Paused){ Label = "Play"; }
    else{ Label = "Pause"; }
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
    //Set the ImGui Button to play the game
    if (ImGui::Button(Label.c_str()))
    {
        if(m_ActiveScene->SceneFile == ""){
            ImGui::OpenPopup("Save Menu");
        }else{
            if(Paused) m_ActiveScene->Save(m_ActiveScene->SceneFile);
            Paused = !Paused;
        }
    }
    if (ImGui::BeginPopup("Save Menu"))
    {
        ImGui::InputText("Scene Name", Name, sizeof(Name), ImGuiInputTextFlags_CharsNoBlank);
        if (ImGui::MenuItem("Save") || glfwGetKey(m_Window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            m_ActiveScene->Save(std::string(Name) + ".scene");
            Paused = !Paused;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
    return IsActive;
}

ImVec2 PlayMode::ScaleWindow(){
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 contentRegionSize = ImGui::GetContentRegionAvail();
    float WindowAspect = windowSize.x / windowSize.y;
    float MainAspect = SCREEN_WIDTH / SCREEN_HEIGHT;
    float adjustedWidth = 0.0f;
    float adjustedHeight = 0.0f;
    if (MainAspect < WindowAspect)
    {
        adjustedHeight = windowSize.y - 20.0f;
        adjustedWidth = adjustedHeight * MainAspect;
    }
    else
    {
        adjustedWidth = windowSize.x - 10.0f;
        adjustedHeight = adjustedWidth / MainAspect;
    }

    ImVec2 textureSize(adjustedWidth, adjustedHeight);
    ImVec2 centerPos = ImVec2((contentRegionSize.x - textureSize.x) / 2.0f, (contentRegionSize.y - textureSize.y) / 2.0f);

    if (MainAspect < WindowAspect)
    {
        centerPos.y += 26.9f;
    }
    else
    {
        centerPos.x += 7.0f;
    }
    ImGui::SetCursorPos(centerPos);
    return textureSize;
}