#include "SceneEditor.h"
#include "Engine/Engine.h"


std::shared_ptr<Object> SceneEditor::OnClick(GLFWwindow* window, std::vector<std::shared_ptr<Object>> Objects, glm::vec2&& WindowPosition)
{  
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        for (size_t i = 0; i < Objects.size(); i++)
        {
            Shapes::Circle* circlePtr = dynamic_cast<Shapes::Circle*>(Objects[i]->GetRenderer()->shape.get());
            //value so I can get the correct mouse position even if the camera is zoomed in/out.
            float scalor = (Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>().x / ViewCamera.Zoom)/Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>().x;
            glm::vec2 CursorPosToWind((CursorPos.x - WindowPosition.x) * scalor - ViewCamera.position.x, -((CursorPos.y - WindowPosition.y) * scalor + ViewCamera.position.y));
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
    FrameBuffer.Init();
    Engine::Get().GetWindows().InitWindow("Viewport");
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


// Its the little offset between the texture's position and the ImGui window's position. I played around with the values and these seem good. Perfect solution I know xD.
constexpr glm::vec2 offset = glm::vec2(7.3f, -6.9f);

// Thank you for the tutorial! https://www.codingwiththomas.com/blog/rendering-an-opengl-framebuffer-into-a-dear-imgui-window
void SceneEditor::Render()
{
    if(!(*Engine::Get().GetWindows().GetWindowState("Viewport"))) return;
    if(!ImGui::Begin("Viewport", Engine::Get().GetWindows().GetWindowState("Viewport"))){
        // ImGui::End();
        // return;
    }

    m_WindowWidth = ImGui::GetContentRegionAvail().x;
    m_WindowHeight = ImGui::GetContentRegionAvail().y;
    Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(m_WindowWidth, m_WindowHeight, 0);


    std::shared_ptr<Object> ClickedObj = OnClick(m_Window, m_ActiveScene->Objects, glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y) + offset);


    
    if(ClickedObj != nullptr && ClickedObj->Name != "MainCamera" && !m_ClickedOnObj && !ImGuizmo::IsUsing())
        SelectedObj = ClickedObj;


    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID*>(FrameBuffer.GetID()), 
        ImVec2(pos.x, pos.y), 
        ImVec2(pos.x + m_WindowWidth, pos.y + m_WindowHeight), 
        ImVec2(0, 1), 
        ImVec2(1, 0)
    );

    // Thanks The Cherno for the amazing tutorial! https://www.youtube.com/watch?v=Pegb5CZuibU
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x + offset.x, ImGui::GetWindowPos().y + offset.y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    
    if(SelectedObj != nullptr){
        glm::vec3& Position = SelectedObj->GetTransform()->Position.value<glm::vec3>();
        glm::vec3& Rotation = SelectedObj->GetTransform()->Rotation.value<glm::vec3>();
        glm::vec3& Scale = SelectedObj->GetTransform()->Size.value<glm::vec3>();

        glm::mat4 proj = glm::ortho(0.0f, ImGui::GetWindowSize().x / ViewCamera.Zoom, 0.0f, ImGui::GetWindowSize().y / ViewCamera.Zoom, -1.0f, 1.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), ViewCamera.position);

        glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
        glm::mat4 Transform = glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);


        if(glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::TRANSLATE;
        else if(glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::ROTATE;
        else if(glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
            m_Operation = ImGuizmo::OPERATION::SCALE;

        ImGuizmo::Manipulate(&view[0][0], &proj[0][0], m_Operation, ImGuizmo::WORLD, &Transform[0][0]);
        if(ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            DecomposeTransform(Transform, translation, rotation, scale);

            if(m_Operation == ImGuizmo::OPERATION::TRANSLATE)
                Position = translation;
            else if(m_Operation == ImGuizmo::OPERATION::SCALE)
                Scale = scale;
            else if(m_Operation == ImGuizmo::OPERATION::ROTATE){
                glm::vec3 deltaRotation = rotation - Rotation;
                Rotation.z += deltaRotation.z;
            }
        }
    }


    MoveCamera(glm::vec2(m_WindowWidth, m_WindowHeight), glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
    glfwSetWindowUserPointer(m_Window, &ViewCamera);
    glfwSetScrollCallback(m_Window, ImGui::IsWindowHovered() ? Zooming : Default);

    ImGui::End();


    FrameBuffer.Bind();

    FrameBuffer.RescaleFrameBuffer(m_WindowWidth, m_WindowHeight);
    GLCall(glViewport(0, 0, m_WindowWidth, m_WindowHeight));
    
    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    m_Grid.Render(ViewCamera.position, ViewCamera.Zoom);
    for (size_t i = 0; i < m_ActiveScene->Objects.size(); i++)
    {
        if(std::shared_ptr<Renderer> renderer = m_ActiveScene->Objects[i]->GetRenderer())
            renderer->Render(m_ActiveScene->Objects[i], m_ActiveScene->Objects[i] == SelectedObj, ViewCamera.position, ViewCamera.Zoom, true);
        else{
            //Check if it does indeed exist and is not set to the renderer variable on the object set it,
            if(m_ActiveScene->Objects[i]->GetRenderer() = m_ActiveScene->Objects[i]->GetComponent<Renderer>()) 
                m_ActiveScene->Objects[i]->GetRenderer()->Render(m_ActiveScene->Objects[i], m_ActiveScene->Objects[i] == SelectedObj, ViewCamera.position, ViewCamera.Zoom, true);
            else
                SapphireEngine::Log(m_ActiveScene->Objects[i]->Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
        }
        m_ActiveScene->Objects[i]->id = i;
    }

    FrameBuffer.Unbind();
}