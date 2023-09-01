#include "Engine.h"
#include "UI/FileExplorer/FileExplorer.h"
#include "Graphics/ShaderFunc.h"
#include "RunTime/RunTime.h"

Engine Engine::Instance;

void ResizeIO(GLFWwindow* window, int width, int height)
{
    Engine::Get().GetWindows().GetWindowIO()->DisplaySize = ImVec2(width,height); // Setting ImGUI to acccess the whole window's place
}
void window_focus_callback(GLFWwindow* window, int focused)
{
    if (focused)
    {
        for (auto&& object : Engine::Get().GetActiveScene()->Objects)
        {
            //Should limit this only to the 
            for(auto&& component : object->GetComponents())
            {
                component->GetLuaVariables(object.get());
            }
        }
    }
}

void Engine::Init(std::string Path)
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);


    //The window is gonna be maximized from the glfw hint above so the width and height are useless
    m_Window = glfwCreateWindow(960, 540, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);
    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(m_Window, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(m_Window);
	glewExperimental = GL_TRUE;
	glViewport(0, 0, bufferWidth, bufferHeight);


    LoadShader(Shapes::CircleShader, "Shaders/Circle.glsl");
    LoadShader(Shapes::BasicShader, "Shaders/Basic.glsl");
    LoadShader(Shapes::GridShader, "Shaders/Grid.glsl");

    m_Windows.Init(std::move(Path));
    m_Viewport.Init(&m_ActiveScene);
    m_PlayMode.Init(&m_ActiveScene);
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

    glfwSetWindowSizeCallback(m_Window, ResizeIO);
    glfwSetWindowFocusCallback(m_Window, window_focus_callback);
    FileExplorer::Init();
    GetActiveScene()->Load("/Test.scene");
    // m_PlayMode.Paused = false;

}

void Engine::Run()
{
    //The main loop.
    while (!glfwWindowShouldClose(m_Window))
    {
        ImGui::SetCurrentContext(m_Windows.GetContext());
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        m_PlayMode.CameraObject->GetTransform()->Size.value<glm::vec3>() = glm::vec3(960, 540, 0);

        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        m_Windows.DockSpace();
        m_Windows.Toolbar();
        FileExplorer::Open(m_Windows.CurrentPath);
        if(m_Viewport.SelectedObj != nullptr) m_Viewport.SelectedObj->Inspect();
        m_Windows.LogWindow();
        m_ActiveScene.Hierechy(m_Viewport.SelectedObj);
        //* The m_Viewport is the actual game scene
        m_PlayMode.Render(m_Windows.MainPath);
        m_Viewport.Render();
        
        ImGui::SetCurrentContext(m_Windows.GetContext());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(m_Window));
        GLCall(glfwPollEvents());
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

Scene *Engine::GetActiveScene()
{
    return &m_ActiveScene;
}

const std::string &Engine::GetMainPath()
{
    return m_Windows.MainPath;
}
