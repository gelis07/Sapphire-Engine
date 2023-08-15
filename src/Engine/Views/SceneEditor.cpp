#include "SceneEditor.h"





std::shared_ptr<Object> SceneEditor::OnClick(GLFWwindow* window, std::vector<std::shared_ptr<Object>> Objects)
{  
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        for (size_t i = 0; i < Objects.size(); i++)
        {
            Shapes::Circle* circlePtr = dynamic_cast<Shapes::Circle*>(Objects[i]->GetRenderer()->shape.get());
            float scalor = (SCREEN_WIDTH / ViewCamera.Zoom)/m_ViewportSize.x;
            glm::vec2 CursorPosToWind((CursorPos.x - m_ViewportPosition.x)* scalor  - ViewCamera.position.x, (m_ViewportPosition.y - CursorPos.y) * scalor - ViewCamera.position.y);
            //Checking if the object is a circle or a rectangle
            if(circlePtr)
            {
                if(sqrt(pow((CursorPosToWind.x- Objects[i]->GetTransform()->Position.value<glm::vec3>().x), 2) + pow((CursorPosToWind.y - Objects[i]->GetTransform()->Position.value<glm::vec3>().y), 2)) < Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2)
                {
                    //The camera can be a little tricky for the time being because its testing for its whole width and height but it should check for a small portion 
                    //At the position so we are skipping it for now
                    if(Objects[i]->Name == "MainCamera")
                        continue;
                    //Object is being clicked!
                    return Objects[i];
                }
            }else if(!circlePtr)
            {

                float Dx = Objects[i]->GetTransform()->Position.value<glm::vec3>().x + Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2; // The x of the bottom left point of the rectangle
                float Cx = Objects[i]->GetTransform()->Position.value<glm::vec3>().x - Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2; // The x of the bottom right point of the rectangle
                float Cy = Objects[i]->GetTransform()->Position.value<glm::vec3>().y - Objects[i]->GetTransform()->Size.value<glm::vec3>().y/2; // The y of the bottom right point of the rectangle
                float By = Objects[i]->GetTransform()->Position.value<glm::vec3>().y + Objects[i]->GetTransform()->Size.value<glm::vec3>().y/2; // The y of the top right point of the rectangle

                if(CursorPosToWind.x < Dx && CursorPosToWind.x > Cx && CursorPosToWind.y < By && CursorPosToWind.y > Cy)
                {
                    if(Objects[i]->Name == "MainCamera")
                        continue;
                    // Object is being clicked!
                    return Objects[i];
                }
            }
        }
    }
    
    return nullptr;
}






void SceneEditor::Init(Scene* activeScene)
{
    m_Window = glfwGetCurrentContext();
    m_ActiveScene = activeScene;
    //Create a framebuffer object to create a texture and render it on the ImGui window
    m_Texture = CreateViewportTexture();
    m_FBO = CreateFBO(m_Texture);

    m_Grid.Init();
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
    m_ViewportPosition = glm::vec2(ImGui::GetWindowPos().x + centerPos.x, ImGui::GetWindowPos().y + centerPos.y + textureSize.y);
    m_ViewportSize = glm::vec2(textureSize.x , textureSize.y);
    ImGui::SetCursorPos(centerPos);
    return textureSize;
}

// This is basically the code for the "Viewport" m_Window
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
void SceneEditor::MoveCamera(){

    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    float scalor = SCREEN_WIDTH/m_ViewportSize.x;
    glm::vec2 CursorPosToWind((CursorPos.x - m_ViewportPosition.x) * scalor, (m_ViewportPosition.y - CursorPos.y) * scalor);
    if(CursorPosToWind.x > 0 && CursorPosToWind.x < SCREEN_WIDTH && CursorPosToWind.y > 0 && CursorPosToWind.y < SCREEN_HEIGHT)
    {
        if(glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(FirstTimeClicking){
                LastPos = CursorPosToWind - glm::vec2(ViewCamera.position);
                FirstTimeClicking = false;
            }
            ViewCamera.position = glm::vec3(CursorPosToWind.x - LastPos.x, CursorPosToWind.y - LastPos.y, 0);
        }
    }
    if(glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && !FirstTimeClicking) FirstTimeClicking = true;

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

    std::string Label;
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
    ImGui::End();
    return IsActive;
}

void SceneEditor::Zooming(GLFWwindow* window, double xoffset, double yoffset){
    ViewportCamera* camera = static_cast<ViewportCamera*>(glfwGetWindowUserPointer(window));
    if(yoffset < 0.0f && camera->Zoom <= 0.1f){
        camera->Zoom = 0.1f;
        return;
    }
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

void SceneEditor::Render()
{
    if(!ImGuiRender(m_Texture)) return;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO)); // Creating a different frame buffer

    std::stringstream ss;
    ss << m_ClickedOnObj;
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);

    std::shared_ptr<Object> ClickedObj = OnClick(m_Window, m_ActiveScene->Objects);

    if(ClickedObj != nullptr && ClickedObj->Name != "MainCamera" && !m_ClickedOnObj)
        SelectedObj = ClickedObj;
    if(glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && ClickedObj != nullptr)
        m_ClickedOnObj = true;
    else if(glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        m_ClickedOnObj = false;

    float scalor = SCREEN_WIDTH/m_ViewportSize.x;
    glm::vec2 CursorPosToWind((CursorPos.x - m_ViewportPosition.x) * scalor  - ViewCamera.position.x, (m_ViewportPosition.y - CursorPos.y) * scalor - ViewCamera.position.y);
    if(m_ClickedOnObj){
        if(m_FirstTime){
            m_Offset = CursorPosToWind - glm::vec2(SelectedObj->GetTransform()->Position.value<glm::vec3>());
            m_FirstTime = false;
        }
        SelectedObj->GetTransform()->Position.AnyValue() = (glm::vec3(CursorPosToWind.x - m_Offset.x, CursorPosToWind.y - m_Offset.y, 0));
    }
    if(!m_ClickedOnObj && !m_FirstTime)
        m_FirstTime = true;

    MoveCamera();
    glfwSetWindowUserPointer(m_Window, &ViewCamera);
    glfwSetScrollCallback(m_Window, IsViewportHovered ? Zooming : Default);
    // HandleObject(SelectedObj, m_ActiveScene->Objects);
    GLCall(glClearColor(0, 0, 0, 1));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    m_Grid.Render(ViewCamera.position, ViewCamera.Zoom);
    for (size_t i = 0; i < m_ActiveScene->Objects.size(); i++)
    {
        if(std::shared_ptr<Renderer> renderer = m_ActiveScene->Objects[i]->GetRenderer())
            renderer->Render(m_ActiveScene->Objects[i] == SelectedObj, ViewCamera.position, ViewCamera.Zoom, true);
        else{
            //Check if it does indeed exist and is not set to the renderer variable on the object set it,
            if(m_ActiveScene->Objects[i]->GetRenderer() = m_ActiveScene->Objects[i]->GetComponent<Renderer>()) 
                m_ActiveScene->Objects[i]->GetRenderer()->Render(m_ActiveScene->Objects[i] == SelectedObj, ViewCamera.position, ViewCamera.Zoom, true);
            else
                SapphireEngine::Log(m_ActiveScene->Objects[i]->Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
        }
        m_ActiveScene->Objects[i]->id = i;
    }
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}