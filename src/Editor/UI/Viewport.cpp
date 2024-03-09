#include "Viewport.h"
#include "Engine/Engine.h"
#include "Editor/Editor.h"
#include "Editor/DebugDraw.h"
#include "UI/FileExplorer/FileExplorer.h"
Viewport::Viewport() : WindowFBO("Viewport"), ViewCamera("Camera", null_ref), SelectedObj(null_ref)
{
    grid.Init();
}

void Viewport::Init()
{
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,-0.5f,0));
    points.push_back(glm::vec3(0.5f,0.5f,0));
    points.push_back(glm::vec3(-0.5f,0.5f,0));
    ViewCamera.Transform = std::make_shared<Transform>("Transform",std::move(points), null_ref);
    ViewCamera.Transform->SetSize(glm::vec3(1 TOUNITS, 1 TOUNITS, 0.0f));

    std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT, null_ref);
    rend->Wireframe = true;
    rend->transform = Engine::GetCameraObject()->GetComponent<Transform>();
    Renderer::Gizmos.push_back(rend);
    window = Engine::app->GetWindow();
}

void Viewport::Draw()
{
    GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    grid.Render(-ViewCamera.Transform->GetPosition() TOPIXELS, ViewCamera.Zoom.Get());
    Renderer::Render(&ViewCamera, Renderer::SceneRenderers);
    Renderer::Render(&ViewCamera, Renderer::Gizmos);
    SapphireEngine::DrawDebug(ViewCamera.GetView());
}
static void Zooming(GLFWwindow* window, double xoffset, double yoffset){
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
void Viewport::Extras()
{
    ViewCamera.Transform->SetSize(glm::vec3(width, height, 0) TOUNITS);
    Engine::GetCameraObject()->GetComponent<Transform>()->SetSize(glm::vec3(width, height, 0) TOUNITS);
    MoveCamera(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y));
    glfwSetWindowUserPointer(window, &ViewCamera);
    glfwSetScrollCallback(window, ImGui::IsWindowHovered() ? Zooming : Default);
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x + offset.x, ImGui::GetWindowPos().y + offset.y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    if (SelectedObj != null_ref)
    {
        Gizmos();
    }
}
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
void Viewport::MoveCamera(glm::vec2 &&Size, glm::vec2 &&Position)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    glm::vec2 CursorPosToWind((CursorPos.x - Position.x), (Position.y - CursorPos.y));
    if(CursorPosToWind.x > 0 && CursorPosToWind.x < Size.x && -CursorPosToWind.y > 0 && -CursorPosToWind.y < Size.y)
    {
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(FirstTimeClicking){
                LastPos = CursorPosToWind + glm::vec2(ViewCamera.Transform->GetPosition() TOPIXELS);
                FirstTimeClicking = false;
            }
            ViewCamera.Transform->SetPosition(-glm::vec3(CursorPosToWind.x - LastPos.x, CursorPosToWind.y - LastPos.y, 0) TOUNITS);
        }
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && !FirstTimeClicking) FirstTimeClicking = true;
}

void Viewport::Gizmos()
{
    const glm::vec3 &Position = SelectedObj->GetComponent<Transform>()->GetPosition() TOPIXELS;
    const glm::vec3 &Rotation = SelectedObj->GetComponent<Transform>()->GetRotation();
    const glm::vec3 &Scale = SelectedObj->GetComponent<Transform>()->GetSize() TOPIXELS;
    glm::mat4 proj = glm::ortho(0.0f, ImGui::GetWindowSize().x / ViewCamera.Zoom.Get(), 0.0f, ImGui::GetWindowSize().y / ViewCamera.Zoom.Get(), -1.0f, 1.0f);

    glm::mat4 model = SelectedObj->GetComponent<Transform>()->GetModel();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        Operation = ImGuizmo::OPERATION::TRANSLATE;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Operation = ImGuizmo::OPERATION::ROTATE;
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        Operation = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::Manipulate(glm::value_ptr(ViewCamera.GetView()), glm::value_ptr(proj), Operation, ImGuizmo::WORLD, glm::value_ptr(model));
    if (ImGuizmo::IsUsing())
    {
        glm::vec3 translation, rotation, scale;
        DecomposeTransform(model, translation, rotation, scale);

        if (Operation == ImGuizmo::OPERATION::TRANSLATE)
            SelectedObj->GetComponent<Transform>()->SetPosition(translation TOUNITS);
        else if (Operation == ImGuizmo::OPERATION::SCALE)
            SelectedObj->GetComponent<Transform>()->SetSize(scale TOUNITS);
        else if (Operation == ImGuizmo::OPERATION::ROTATE)
        {
            glm::vec3 deltaRotation = rotation - Rotation;
            SelectedObj->GetComponent<Transform>()->Rotate(deltaRotation.z);
        }
    }
}

bool Viewport::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
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

void Hierachy::Display()
{
    for (size_t i = 0; i < Engine::GetActiveScene().Objects.size(); i++)
    {
        ObjectRef obj = Engine::GetActiveScene().Objects[i].GetRef();
        if(obj->Parent != null_ref) continue;
        std::string Name = "";
        Name = obj->Name.c_str();
        if (Name.empty())
        {
            Name = "##";
        }
        if(obj->Children.size() == 0 && obj->Parent == null_ref){
            ImVec2 textSize = ImGui::CalcTextSize(Name.c_str());
            if (ImGui::Selectable((Name + "##" + std::to_string(obj->id)).c_str(), obj == viewport->SelectedObj))
            {
                viewport->SelectedObj = obj;
                Editor::SelectedObjChildID = -1;
            }
            if(std::shared_ptr<ObjectRef>* ref = HierachyDrop.ReceiveDropLoop(glm::vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y), glm::vec2(textSize.x, textSize.y))){
                if((**ref) != obj->GetRefID()){
                    obj->Children.push_back(**ref);
                    (**ref)->Parent = obj->GetRef();
                    (**ref)->GetComponent<Transform>()->TransParent = obj->GetComponent<Transform>().get();
                    obj->GetComponent<Transform>()->childrenTransforms.push_back((**ref)->GetComponent<Transform>().get());
                    (**ref)->GetComponent<Transform>()->UpdateModel();
                }
            }
            if(ImGui::IsItemClicked(0)){
                HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(obj->GetRefID()));
            }
        }else{
            if(ImGui::TreeNode((Name + "##" + std::to_string(obj->id)).c_str())){
                ImVec2 textSize = ImGui::CalcTextSize(Name.c_str());
                if(std::shared_ptr<ObjectRef>* ref = HierachyDrop.ReceiveDropLoop(glm::vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y), glm::vec2(textSize.x, textSize.y))){
                    if((**ref) != obj->GetRefID()){
                        obj->Children.push_back(**ref);
                        (**ref)->Parent = obj->GetRef();
                        (**ref)->GetComponent<Transform>()->TransParent = obj->GetComponent<Transform>().get();
                        obj->GetComponent<Transform>()->childrenTransforms.push_back((**ref)->GetComponent<Transform>().get());
                        (**ref)->GetComponent<Transform>()->UpdateModel();
                    }
                }
                if(ImGui::IsItemClicked(0)){
                    viewport->SelectedObj = obj;
                    Editor::SelectedObjChildID = -1;
                    HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(obj->GetRefID()));
                }
                for (size_t j = 0; j < obj->Children.size(); j++)
                {
                    if (ImGui::Selectable((obj->Children[j]->Name + "##" + std::to_string(obj->Children[j]->id)).c_str(), obj == viewport->SelectedObj))
                    {
                        viewport->SelectedObj = obj->Children[j];
                    }
                    if(ImGui::IsItemClicked(0)){
                        HierachyDrop.StartedDragging(std::make_shared<ObjectRef>(obj->Children[j]));
                    }
                }
                ImGui::TreePop();
            }
        }
    }
    ImVec2 textSize = ImGui::CalcTextSize(Engine::GetActiveScene().Objects[0].Name.c_str());
    if(std::shared_ptr<ObjectRef>* ref = HierachyDrop.ReceiveDropLoop(glm::vec2(ImGui::GetWindowPos().x, textSize.y * Engine::GetActiveScene().Objects.size()), 
    glm::vec2(ImGui::GetWindowPos().x +ImGui::GetWindowSize().x , ImGui::GetWindowPos().y +ImGui::GetWindowSize().y))){
        // if((**ref)->Parent != null_ref){
        //     for (size_t i = 0; i < (**ref)->Parent->Children.size(); i++)
        //     {
                
        //         if((**ref)->Parent->Children[i].Get() == (**ref).Get()){
        //             (**ref)->Parent->GetComponent<Transform>()->childrenTransforms.erase((**ref)->Parent->GetComponent<Transform>()->childrenTransforms.begin() + i);
        //             (**ref)->Parent->Children.erase((**ref)->Parent->Children.begin() + i);
        //         }
        //     }
        // }
        // (**ref)->Parent = null_ref;
        // (**ref)->GetComponent<Transform>()->TransParent = nullptr;
    }
    if (Engine::app->GetInputDown(GLFW_KEY_DELETE) && ImGui::IsWindowFocused())
    {
        for (size_t i = 0; i < Engine::GetActiveScene().Objects.size(); i++)
        {
            if (Engine::GetActiveScene().Objects[i].Name == viewport->SelectedObj->Name)
            {
                Object::Delete(i);
                if(Editor::SelectedObjID >= Engine::GetActiveScene().Objects.size()){
                    // Editor::SelectedObjID = Engine::GetActiveScene().Objects.size() - 1;
                }else if(Engine::GetActiveScene().Objects.size() == 0){
                    // Editor::SelectedObjID = -1;
                }
            }
        }
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context Menu");
    }
    CreateMenu(viewport->SelectedObj);
}

void Hierachy::CreateMenu(ObjectRef SelectedObj)
{
    Scene* scene = &Engine::GetActiveScene();
    if (ImGui::BeginPopup("Context Menu"))
    {
        if (ImGui::MenuItem("Create Empty"))
        {
            std::stringstream ss;
            ss << "Object: " << scene->Objects.size();
            Object Obj = Object(ss.str());
            scene->Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Circle"))
        {
            std::stringstream ss;
            ss << "Object: " << scene->Objects.size();
            Object Obj = Object(ss.str());
            Obj.AddComponent((std::make_shared<Renderer>(SapphireRenderer::CircleShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::CircleT, Obj.GetRef())));
            Obj.GetComponent<Renderer>()->transform = Obj.GetComponent<Transform>();
            Renderer* rend = Obj.GetComponent<Renderer>().get();
            Obj.GetComponent<Renderer>()->SetUpUniforms = [rend](SapphireRenderer::Shader& shader, Camera* cam) { 
                const glm::vec3& ObjectSize = rend->transform->GetSize() TOPIXELS;
                const glm::vec3& ObjectPos = rend->transform->GetPosition() TOPIXELS;
                glm::vec3 CamPos = (-cam->Transform->GetPosition() + cam->Transform->GetSize() / 2.0f) TOPIXELS;
                glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CamPos.x, ObjectPos.y - ObjectSize.y/2 + CamPos.y);
                shader.SetUniform("RectWidth", ObjectSize.x);
                shader.SetUniform("RectHeight", ObjectSize.y);
                shader.SetUniform("StartPoint", StartPos);
                shader.SetUniform("CameraZoom", cam->Zoom.Get());
            };
            Renderer::SceneRenderers.push_back(Obj.GetComponent<Renderer>());
            Obj.GetComponent<Renderer>()->Color.Get() = glm::vec4(1);
            ObjectRef objRef = scene->Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Rectangle"))
        {
            std::stringstream ss;
            ss << "Object: " << scene->Objects.size();
            Object Obj = Object(ss.str());
            Obj.AddComponent((std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT, Obj.GetRef())));
            Obj.GetComponent<Renderer>()->transform = Obj.GetComponent<Transform>();
            Renderer::SceneRenderers.push_back(Obj.GetComponent<Renderer>());
            Obj.GetComponent<Renderer>()->Color.Get() = glm::vec4(1);
            scene->Add(std::move(Obj));
        }
        if (ImGui::MenuItem("Create Sprite"))
        {
            std::stringstream ss;
            ss << "Object: " << scene->Objects.size();
            Object Obj = Object(ss.str());
            Obj.GetComponent<Renderer>()->ShapeType = SapphireRenderer::RectangleT;
            Obj.GetComponent<SapphirePhysics::RigidBody>()->ShapeType = static_cast<int>(Obj.GetComponent<Renderer>()->ShapeType);
            scene->Add(std::move(Obj));
        }
        ImGui::EndPopup();
    }
}