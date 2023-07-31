#include "Engine.h"
#include "UI/FileExplorer/FileExplorer.h"
#include "Graphics/ShaderFunc.h"

Scene* ActiveScenePtr = nullptr;
std::string MainPath = "";

//This could be called as the "Start" function for the engine.
Engine::Engine(std::string Path)
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    MainWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(MainWindow);
    glfwSwapInterval(1);
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


    LoadShader(Shapes::CircleShader, "Shaders/Circle.glsl");
    LoadShader(Shapes::BasicShader, "Shaders/Basic.glsl");
    LoadShader(Shapes::GridShader, "Shaders/Grid.glsl");

    windows.Init(std::move(Path));
    viewport.Init(&ActiveScene);
    playMode.Init(&ActiveScene);
    MainPath = windows.MainPath;
    ActiveScenePtr = &ActiveScene;
    ImGui_ImplGlfw_InitForOpenGL(MainWindow, true);
    ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));


    if(!EXPORT) glfwMaximizeWindow(MainWindow);
    if(!EXPORT) FileExplorer::Init();
}

void Engine::Run()
{
    //The main loop.
    while (!glfwWindowShouldClose(MainWindow))
    {
        ImGui::SetCurrentContext(windows.GetContext());
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        int NewWidth, NewHeight; 
        glfwGetWindowSize(MainWindow, &NewWidth, &NewHeight); // Getting the size of the window
        windows.GetWindowIO()->DisplaySize = ImVec2(NewWidth, NewHeight); // Setting ImGUI to acccess the whole window's place
        
        if(!EXPORT) windows.DockSpace();
        if(!EXPORT) FileExplorer::Open(windows.MainPath);
        if(viewport.SelectedObj != nullptr)viewport.SelectedObj->Inspect();
        if(!EXPORT) windows.LogWindow();
        if(!EXPORT) ActiveScene.Hierechy(viewport.SelectedObj);

        //* The viewport is the actual game scene
        if(!EXPORT) viewport.Render();
        if(!EXPORT) playMode.Render(windows.MainPath);
        ImGui::SetCurrentContext(windows.GetContext());
        if(!EXPORT) GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        if(!EXPORT) GLCall(glClear(GL_COLOR_BUFFER_BIT));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(MainWindow));
        GLCall(glfwPollEvents());
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

const std::string &GetMainPath()
{
    return MainPath;
}

Scene* GetActiveScene() {
    return ActiveScenePtr;
}