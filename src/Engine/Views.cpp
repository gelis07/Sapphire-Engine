#include "Views.h"

void Viewport::Views::OnApplicationStart(){
    // MainCamera->AddComponentTemp<Transform>();
    // MainCamera->AddComponentTemp<Camera>();
    // CurrentScene->Objects.push_back(MainCamera);
    // Events::viewport = this;

    //Create a framebuffer object to create a texture and render it on the ImGui window
    if(!EXPORT) ViewportTexture = CreateViewportTexture();
    if(!EXPORT) ViewportFBO = CreateFBO(ViewportTexture);

    if(!EXPORT) PlayWindowTexture = CreateViewportTexture();
    if(!EXPORT) PlayWindowFBO = CreateFBO(PlayWindowTexture);
}

ImVec2 Viewport::Views::ScaleWindow(){
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
    ViewportPoint = glm::vec2(ImGui::GetWindowPos().x + centerPos.x, ImGui::GetWindowPos().y + centerPos.y + textureSize.y);
    TextureSize = glm::vec2(textureSize.x , textureSize.y);
    ImGui::SetCursorPos(centerPos);
    return textureSize;
}

// This is basically the code for the "Viewport" Window
void Viewport::Views::SceneEditor()
{
    if(!ViewportRender(ViewportTexture)) return;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ViewportFBO)); // Creating a different frame buffer

    std::stringstream ss;
    ss << ClickedOnObj;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);

    if(!ClickedOnObj && !FirstTime) FirstTime = true;

    // HandleObject(SelectedObj, CurrentScene->Objects);
    GLCall(glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, BackgroundColor.a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    try {
        // ShowGrid();
        for (size_t i = 0; i < Viewport::CurrentScene->Objects.size(); i++)
        {
            Viewport::CurrentScene->Objects[i]->GetComponent<Renderer>()->Render(CurrentScene->Objects[i].get(),window, Shapes::ViewportCamera.position, SelectedObj, true);
            // SelectedObj = nullptr;
            CurrentScene->Objects[i]->id = i;
        }
    }
    catch (const char* msg) {
        Paused = true;
        Log(std::string(msg), Utilities::Error);
    } 
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
char Name[256] = "";

bool Viewport::Views::PlayWindowRender(unsigned int texture)
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
        if(CurrentScene->SceneFile == ""){
            ImGui::OpenPopup("Save Menu");
        }else{
            if(Paused) CurrentScene->Save(CurrentScene->SceneFile);
            Paused = !Paused;
        }
    }
    if (ImGui::BeginPopup("Save Menu"))
    {
        ImGui::InputText("Scene Name", Name, sizeof(Name), ImGuiInputTextFlags_CharsNoBlank);
        if (ImGui::MenuItem("Save"))
        {
            CurrentScene->Save(std::string(Name) + ".scene");
            Paused = !Paused;
        }
        ImGui::EndPopup();
    }
    ImGui::End();
    return IsActive;
}
bool IsViewportHovered = false;
bool Viewport::Views::ViewportRender(unsigned int texture)
{
    bool IsActive = ImGui::Begin("Viewport");
    IsViewportHovered = ImGui::IsWindowHovered();
    ImTextureID textureID = (void *)(intptr_t)texture;
    ImVec2 uv0 = ImVec2(0, 1);
    ImVec2 uv1 = ImVec2(1, 0);
    ImGui::Image(textureID, ScaleWindow(), uv0, uv1);

    std::string Label;
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
    ImGui::End();
    return IsActive;
}
void Viewport::Views::PlayWindow(){
    if(!PlayWindowRender(PlayWindowTexture)) return;
    if(!EXPORT) GLCall(glBindFramebuffer(GL_FRAMEBUFFER, PlayWindowFBO));

    std::stringstream ss;
    ss << ClickedOnObj;
    GLCall(glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, BackgroundColor.a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(Paused && !Start){
        //This indicates that the game has been paused and we should reset the start boolean so next time the user hits play
        //The start functions get called
        SelectedObj = nullptr;
        CurrentScene->Load(CurrentScene->SceneFile, Viewport::window);
        Start = true;
    } 
    try {
        for (size_t i = 0; i < CurrentScene->Objects.size(); i++)
        {
            Viewport::CurrentScene->Objects[i]->GetComponent<Renderer>()->Render(CurrentScene->Objects[i].get(),window, Shapes::ViewportCamera.position, nullptr, false);
            if(!Paused){
                CurrentScene->Objects[i]->OnStart();
                CurrentScene->Objects[i]->OnUpdate();
                // CurrentScene->Objects[i]->Run();
                // CurrentScene->Objects[i]->GetComponent<Physics>()->CheckCollision(CurrentScene->Objects);
            }
        }
        //Changing the start bool to false here so all of the start functions get executed
        if(!Paused) Start = false;
    }
    catch (const char* msg) {
        Paused = true;
        Log(std::string(msg), Utilities::Error);
    } 
    if(!EXPORT) GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}