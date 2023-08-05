#include "Engine.h"
#include "UI/FileExplorer/FileExplorer.h"
#include "Graphics/ShaderFunc.h"

Engine Engine::Instance;

void Engine::Init(std::string Path)
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    m_Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


    LoadShader(Shapes::CircleShader, "Shaders/Circle.glsl");
    LoadShader(Shapes::BasicShader, "Shaders/Basic.glsl");
    LoadShader(Shapes::GridShader, "Shaders/Grid.glsl");

    m_Windows.Init(std::move(Path));
    m_Viewport.Init(&m_ActiveScene);
    m_PlayMode.Init(&m_ActiveScene);
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));


    glfwMaximizeWindow(m_Window);
    FileExplorer::Init();
}

void Engine::Run()
{
    //The main loop.
    while (!glfwWindowShouldClose(m_Window))
    {
        ImGui::SetCurrentContext(m_Windows.GetContext());
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        int NewWidth, NewHeight; 
        glfwGetWindowSize(m_Window, &NewWidth, &NewHeight); // Getting the size of the window
        m_Windows.GetWindowIO()->DisplaySize = ImVec2(NewWidth, NewHeight); // Setting ImGUI to acccess the whole window's place
        
        m_Windows.DockSpace();
        m_Windows.Toolbar();
        FileExplorer::Open(m_Windows.MainPath);
        if(m_Viewport.SelectedObj != nullptr) m_Viewport.SelectedObj->Inspect();
        m_Windows.LogWindow();
        m_ActiveScene.Hierechy(m_Viewport.SelectedObj);

        //* The m_Viewport is the actual game scene
        m_Viewport.Render();
        m_PlayMode.Render(m_Windows.MainPath);
        ImGui::SetCurrentContext(m_Windows.GetContext());
        GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
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