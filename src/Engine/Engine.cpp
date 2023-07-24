#include "Engine.h"
#include "UI/FileExplorer/FileExplorer.h"
Engine::Engine(std::string Path)
{
    Windows::Init(Path);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    Viewport::window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(Viewport::window);
    glfwSwapInterval(1);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    LoadShader(Shapes::CircleShader, "Shaders/Circle.glsl");
    LoadShader(Shapes::BasicShader, "Shaders/Basic.glsl");
    LoadShader(Shapes::GridShader, "Shaders/Grid.glsl");

    ImGui_ImplGlfw_InitForOpenGL(Viewport::window, true);
    ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    viewport.OnApplicationStart();
    if(!EXPORT) glfwMaximizeWindow(Viewport::window);
}

void Engine::Run()
{
    while (!glfwWindowShouldClose(Viewport::window))
    {
        ImGui::SetCurrentContext(Windows::DefaultContext);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float currentTime = glfwGetTime();
        Viewport::DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        int NewWidth, NewHeight; 
        glfwGetWindowSize(Viewport::window, &NewWidth, &NewHeight); // Getting the size of the window
        Windows::IO->DisplaySize = ImVec2(NewWidth, NewHeight); // Setting ImGUI to acccess the whole window's place
        
        if(!EXPORT) Windows::DockSpace();
        if(!EXPORT) FileExplorer::Open(Windows::MainPath);
        if(viewport.SelectedObj != nullptr)viewport.SelectedObj->Inspect();
        if(!EXPORT) Windows::LogWindow();
        if(!EXPORT) Viewport::CurrentScene->Hierechy(viewport.SelectedObj);

        //* The viewport is the actual game scene
        if(!EXPORT) viewport.SceneEditor();
        if(!EXPORT) viewport.PlayWindow();
        ImGui::SetCurrentContext(Windows::DefaultContext);
        if(!EXPORT) GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        if(!EXPORT) GLCall(glClear(GL_COLOR_BUFFER_BIT));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(Viewport::window));
        GLCall(glfwPollEvents());
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}