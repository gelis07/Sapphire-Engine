#include "PlayMode.h"

void PlayMode::Init(Scene* activeScene)
{
    Window = glfwGetCurrentContext();
    ActiveScene = activeScene;
    Texture = CreateViewportTexture();
    FBO = CreateFBO(Texture);
}

void PlayMode::Render(std::string& MainPath) //Here we need the main path because we load the scene after the user hops out of play mode
{
    if(!ImGuiRender(Texture, MainPath)) return;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, FBO));

    GLCall(glClearColor(0, 0, 0, 1));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(Paused && !Start){
        //This indicates that the game has been paused and we should reset the start boolean so next time the user hits play
        //The start functions get called
        ActiveScene->Load(ActiveScene->SceneFile, MainPath,Window);
        Start = true;
    } 
    try {
        for (size_t i = 0; i < ActiveScene->Objects.size(); i++)
        {
            ActiveScene->Objects[i]->GetComponent<Renderer>()->Render(false, glm::vec3(0), 1.0f,false);
            if(!Paused){
                ActiveScene->Objects[i]->OnStart();
                ActiveScene->Objects[i]->OnUpdate();
            }
        }
        //Changing the start bool to false here so all of the start functions get executed
        if(!Paused) Start = false;
    }
    catch (const char* msg) {
        Paused = true;
        Log(std::string(msg), SapphireEngine::Error);
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
        if(ActiveScene->SceneFile == ""){
            ImGui::OpenPopup("Save Menu");
        }else{
            if(Paused) ActiveScene->Save(ActiveScene->SceneFile, MainPath);
            Paused = !Paused;
        }
    }
    if (ImGui::BeginPopup("Save Menu"))
    {
        ImGui::InputText("Scene Name", Name, sizeof(Name), ImGuiInputTextFlags_CharsNoBlank);
        if (ImGui::MenuItem("Save") || glfwGetKey(Window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            ActiveScene->Save(std::string(Name) + ".scene", MainPath);
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