#include "RunTime.h"
#include "Engine/Engine.h"
#include "Graphics/ShaderFunc.h"

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


    LoadShader(Shapes::CircleShader, "Shaders/Circle.glsl");
    LoadShader(Shapes::BasicShader, "Shaders/Basic.glsl");
    LoadShader(Shapes::GridShader, "Shaders/Grid.glsl");

    // Scene->Load("/Test.scene");
}

void RunTime::Render(std::shared_ptr<Object> object, std::shared_ptr<Object> CameraObject)
{
    if(std::shared_ptr<Renderer> renderer = object->GetRenderer()) {
        if(object != CameraObject)
            renderer->Render(object,false, -CameraObject->GetTransform()->Position.value<glm::vec3>(),CameraObject->GetComponent<Camera>()->Zoom.value<float>(), false);
    }
    else
        if(object->GetRenderer() = object->GetComponent<Renderer>()) 
            object->GetRenderer()->Render(object,false, -CameraObject->GetTransform()->Position.value<glm::vec3>(),CameraObject->GetComponent<Camera>()->Zoom.value<float>(), false);
        else
            SapphireEngine::Log(object->Name + " (Object) doesn't have a renderer component attached!", SapphireEngine::Error);
}

void RunTime::Run(Scene* Scene, std::shared_ptr<Object>& CameraObject, const float& DeltaTime)
{
    for (size_t i = 0; i < Scene->Objects.size(); i++)
    {
        Render(Scene->Objects[i], CameraObject);
        Scene->Objects[i]->OnStart();
        Scene->Objects[i]->OnUpdate();
        if(std::shared_ptr<RigidBody> rb = Scene->Objects[i]->GetComponent<RigidBody>()) {
            rb->Simulate(Scene->Objects[i].get(), DeltaTime);
            rb->CheckForCollisions(Scene->Objects[i].get());
        }
    }
}

void RunTime::RunGame(GLFWwindow* window,Scene *Scene, std::shared_ptr<Object>& CameraObject)
{

    //The main loop.
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
        Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(width, height, 0);
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
