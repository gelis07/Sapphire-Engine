#include "SceneEditor.h"




// std::shared_ptr<Object> SceneEditor::OnClick(GLFWwindow* window, std::vector<std::shared_ptr<Object>> Objects)
// {  
//     double xpos, ypos;
//     glfwGetCursorPos(window, &xpos, &ypos);
//     glm::vec2 CursorPos(xpos, ypos);
//     if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
//     {
//         for (size_t i = 0; i < Objects.size(); i++)
//         {
//             Shapes::Circle* circlePtr = dynamic_cast<Shapes::Circle*>(Objects[i]->GetRenderer()->shape.get());
//             float scalor = (SCREEN_WIDTH / ViewCamera.Zoom)/m_ViewportSize.x;
//             glm::vec2 CursorPosToWind((CursorPos.x - m_ViewportPosition.x)* scalor  - ViewCamera.position.x, (m_ViewportPosition.y - CursorPos.y) * scalor - ViewCamera.position.y);
//             //Checking if the object is a circle or a rectangle
//             if(circlePtr)
//             {
//                 if(sqrt(pow((CursorPosToWind.x- Objects[i]->GetTransform()->Position.value<glm::vec3>().x), 2) + pow((CursorPosToWind.y - Objects[i]->GetTransform()->Position.value<glm::vec3>().y), 2)) < Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2)
//                 {
//                     //The camera can be a little tricky for the time being because its testing for its whole width and height but it should check for a small portion 
//                     //At the position so we are skipping it for now
//                     if(Objects[i]->Name == "MainCamera")
//                         continue;
//                     //Object is being clicked!
//                     return Objects[i];
//                 }
//             }else if(!circlePtr)
//             {

//                 float Dx = Objects[i]->GetTransform()->Position.value<glm::vec3>().x + Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2; // The x of the bottom left point of the rectangle
//                 float Cx = Objects[i]->GetTransform()->Position.value<glm::vec3>().x - Objects[i]->GetTransform()->Size.value<glm::vec3>().x/2; // The x of the bottom right point of the rectangle
//                 float Cy = Objects[i]->GetTransform()->Position.value<glm::vec3>().y - Objects[i]->GetTransform()->Size.value<glm::vec3>().y/2; // The y of the bottom right point of the rectangle
//                 float By = Objects[i]->GetTransform()->Position.value<glm::vec3>().y + Objects[i]->GetTransform()->Size.value<glm::vec3>().y/2; // The y of the top right point of the rectangle

//                 if(CursorPosToWind.x < Dx && CursorPosToWind.x > Cx && CursorPosToWind.y < By && CursorPosToWind.y > Cy)
//                 {
//                     if(Objects[i]->Name == "MainCamera")
//                         continue;
//                     // Object is being clicked!
//                     return Objects[i];
//                 }
//             }
//         }
//     }
    
//     return nullptr;
// }



void SceneEditor::Init(Scene* activeScene)
{
    m_Window = glfwGetCurrentContext();
    m_ActiveScene = activeScene;
    //Create a framebuffer object to create a texture and render it on the ImGui window
    m_Texture = CreateViewportTexture();
    m_FBO = CreateFBO(m_Texture);

    m_Grid.Init();
}

// This is basically the code for the "Viewport" m_Window
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
void SceneEditor::MoveCamera(glm::vec2&& Size, glm::vec2&& Position){

    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    glm::vec2 CursorPosToWind((CursorPos.x - Position.x), (Position.y - CursorPos.y));
    if(CursorPosToWind.x > 0 && CursorPosToWind.x < Size.x && -CursorPosToWind.y > 0 && -CursorPosToWind.y < Size.y)
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

void SceneEditor::RescaleFrameBuffer(float width, float height)
{
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);
}


// Thank you for the tutorial! https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
void SceneEditor::Render()
{

    ImGui::Begin("Viewport");

    m_WindowWidth = ImGui::GetContentRegionAvail().x;
    m_WindowHeight = ImGui::GetContentRegionAvail().y;

    RescaleFrameBuffer(m_WindowWidth, m_WindowHeight);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID*>(m_Texture), 
        ImVec2(pos.x, pos.y), 
        ImVec2(pos.x + m_WindowWidth, pos.y + m_WindowHeight), 
        ImVec2(0, 1), 
        ImVec2(1, 0)
    );


    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    
    glm::mat4 proj = glm::ortho(0.0f, ImGui::GetWindowSize().x, 0.0f, ImGui::GetWindowSize().y, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), ViewCamera.position);
    model = glm::translate(model, glm::vec3(0, 0, 0));
    ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, &model[0][0]);


    MoveCamera(glm::vec2(m_WindowWidth, m_WindowHeight), glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
    glfwSetWindowUserPointer(m_Window, &ViewCamera);
    glfwSetScrollCallback(m_Window, ImGui::IsWindowHovered() ? Zooming : Default);

    ImGui::End();


    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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