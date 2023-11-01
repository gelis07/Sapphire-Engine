#include "RunTime.h"
#include "Engine/Engine.h"

void RunTime::Init(GLFWwindow* window,Scene* Scene, std::shared_ptr<Object>& CameraObject)
{

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);




    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;


    SapphireRenderer::LoadShader(const_cast<GLuint&>(Shapes::CircleShader.GetID()), "Shaders/Circle.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(Shapes::BasicShader.GetID()), "Shaders/Basic.glsl");

    // Scene->Load("/Test.scene");
}

void RunTime::Render(std::shared_ptr<Object> object, std::shared_ptr<Object> CameraObject)
{
    glm::mat4 view = glm::translate(glm::mat4(1.0f), CameraObject->GetTransform()->GetPosition() + CameraObject->GetTransform()->GetSize() / 2.0f);
    if(std::shared_ptr<Renderer> renderer = object->GetRenderer()) {
        if(object != CameraObject)
            renderer->Render(*object->GetTransform(),view, false, -CameraObject->GetTransform()->GetPosition(), 1.0f);
    }
    else
        if(object->GetRenderer() = object->GetComponent<Renderer>()) 
            renderer->Render(*object->GetTransform(),view, false, -CameraObject->GetTransform()->GetPosition(), 1.0f);
        else
            SapphireEngine::Log(object->Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
}
#define CheckForSkip(x) \
    x; \
    if(RunTime::SkipFrame){ \
        break; \
    } \

void RunTime::Run(Scene* Scene, std::shared_ptr<Object>& CameraObject, const float& DeltaTime)
{
    // for (size_t Obj1 = 0; Obj1 < Scene->Objects.size(); Obj1++)
    // {
    //     for (size_t Obj2 = Obj1; Obj2< Scene->Objects.size(); Obj2++)
    //     {
    //         glm::vec2 Normal;
    //         float Depth;
    //         if(Scene->Objects[Obj1]->GetRenderer()->shape->ShapeType == Shapes::RectangleT){
    //            if(Scene->Objects[Obj2]->Name == "MainCamera" || Scene->Objects[Obj2] == Scene->Objects[Obj1]) continue;
    //             if(Scene->Objects[Obj2]->GetComponent<Renderer>()->shape->ShapeType == Shapes::RectangleT){
    //                 CollisionData CD;
    //                 if(SapphirePhysics::CollisionDetection::RectanglexRectangle(Scene->Objects[Obj2], Scene->Objects[Obj1].get(),CD)){
    //                     Scene->Objects[Obj1]->GetComponent<RigidBody>()->rb.OnCollisionRotation(Scene->Objects[Obj1].get(), Scene->Objects[Obj2].get(), std::move(CD));
    //                     break;
    //                 }
    //             }
    //         }
    //     }
    // }
    for (size_t i = 0; i < Scene->Objects.size(); i++)
    {
        Render(Scene->Objects[i], CameraObject);
        if(Engine::Get().GetPlay().Paused) continue;
        CheckForSkip(Scene->Objects[i]->OnStart());
        CheckForSkip(Scene->Objects[i]->OnUpdate());
        if(std::shared_ptr<SapphirePhysics::RigidBody> rb = Scene->Objects[i]->GetComponent<SapphirePhysics::RigidBody>()) {
            rb->Simulate(Scene->Objects[i].get(), DeltaTime);
            //Currently collision checks for object's twice but I'm planning on adding multithreading where all of these things will happen at the same time.
            //So I'm leaving this for now, and I'm gonna fix this when I get more into optimizing this engine.
            // CheckForSkip(rb->CheckForCollisions(Scene->Objects[i].get()));
        }

    }
    RunTime::SkipFrame = false;
    Time += DeltaTime;
}

void RunTime::RunGame(GLFWwindow* window,Scene *Scene, std::shared_ptr<Object>& CameraObject)
{

    //The main loop.
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
        Engine::Get().GetPlay().CameraObject->GetTransform()->SetSize(glm::vec3(width, height, 0));
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        float currentTime = glfwGetTime();
        RunTime::DeltaTime = currentTime - RunTime::LastTime;
        RunTime::LastTime = currentTime;


        RunTime::Run(Scene, CameraObject, RunTime::DeltaTime);

        GLCall(glfwSwapBuffers(window));
        GLCall(glfwPollEvents());
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}
