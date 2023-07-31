#include "SceneEditor.h"


void SceneEditor::Init(Scene* activeScene)
{
    Window = glfwGetCurrentContext();
    ActiveScene = activeScene;
    //Create a framebuffer object to create a texture and render it on the ImGui window
    Texture = CreateViewportTexture();
    FBO = CreateFBO(Texture);

    grid.Init();
}

ImVec2 SceneEditor::ScaleWindow(){
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
    ViewportPos = glm::vec2(ImGui::GetWindowPos().x + centerPos.x, ImGui::GetWindowPos().y + centerPos.y + textureSize.y);
    TextureSize = glm::vec2(textureSize.x , textureSize.y);
    ImGui::SetCursorPos(centerPos);
    return textureSize;
}

// This is basically the code for the "Viewport" Window
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
void SceneEditor::MoveCamera(){

    double xpos, ypos;
    glfwGetCursorPos(Window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    float scalor = SCREEN_WIDTH/TextureSize.x;
    glm::vec2 CursorPosToWind((CursorPos.x - ViewportPos.x) * scalor, (ViewportPos.y - CursorPos.y) * scalor);
    if(CursorPosToWind.x > 0 && CursorPosToWind.x < SCREEN_WIDTH && CursorPosToWind.y > 0 && CursorPosToWind.y < SCREEN_HEIGHT)
    {
        if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(FirstTimeClicking){
                LastPos = CursorPosToWind - glm::vec2(ViewportCamera.position);
                FirstTimeClicking = false;
            }
            ViewportCamera.position = glm::vec3(CursorPosToWind.x - LastPos.x, CursorPosToWind.y - LastPos.y, 0);
        }
    }
    if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && !FirstTimeClicking) FirstTimeClicking = true;

}

bool IsViewportHovered = false;
bool SceneEditor::ImGuiRender(unsigned int texture)
{
    bool IsActive = ImGui::Begin("Viewport");
    IsViewportHovered = ImGui::IsWindowHovered();
    ImTextureID textureID = (void *)(intptr_t)texture;
    ImVec2 uv0 = ImVec2(0, 1);
    ImVec2 uv1 = ImVec2(1, 0);
    ImGui::Image(textureID, ScaleWindow(), uv0, uv1);

    // ImGuizmo::SetOrthographic(true);
    // ImGuizmo::SetDrawlist();

    // ImGuizmo::SetRect(ViewportCamera.position.x, ViewportCamera.position.y, TextureSize.x, TextureSize.y);
    
    // glm::mat4 proj = glm::ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -1.0f, 1.0f);
    // glm::mat4 model = glm::mat4(1.0f);
    // glm::mat4 view = glm::translate(glm::mat4(1.0f), ViewportCamera.position);
    // model = glm::translate(model, glm::vec3(100, 100, 0));
    // ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, &model[0][0]);

    std::string Label;
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
    ImGui::End();
    return IsActive;
}

void SceneEditor::Zooming(GLFWwindow* window, double xoffset, double yoffset){
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if(yoffset < 0.0f && camera->Zoom <= 0.1f){
        camera->Zoom = 0.1f;
        return;
    }
    camera->Zoom += yoffset/10;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
    delete(camera);
}

static void Default(GLFWwindow* window, double xoffset, double yoffset){
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);
}

void SceneEditor::Render()
{
    if(!ImGuiRender(Texture)) return;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, FBO)); // Creating a different frame buffer

    std::stringstream ss;
    ss << ClickedOnObj;
    double xpos, ypos;
    glfwGetCursorPos(Window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);

    if(!ClickedOnObj && !FirstTime) FirstTime = true;
    MoveCamera();
    glfwSetWindowUserPointer(Window, &ViewportCamera);
    glfwSetScrollCallback(Window, IsViewportHovered ? Zooming : Default);
    // HandleObject(SelectedObj, ActiveScene->Objects);
    GLCall(glClearColor(0, 0, 0, 1));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    grid.Render(ViewportCamera.position, ViewportCamera.Zoom);
    for (size_t i = 0; i < ActiveScene->Objects.size(); i++)
    {
        ActiveScene->Objects[i]->GetComponent<Renderer>()->Render(ActiveScene->Objects[i] == SelectedObj, ViewportCamera.position, ViewportCamera.Zoom, true);
        //^ Should store the renderer on a variable and not search for it every frame
        ActiveScene->Objects[i]->id = i;
    }
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}