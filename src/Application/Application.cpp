#include "Application.h"

Application::Application(const std::string& Path)
{

    if(Path == ""){
        AppMainPath = std::filesystem::current_path().string();
    }else{
        if(std::filesystem::exists(Path)){
            AppMainPath = Path;
        }else{
            std::cout << "Path not found" << '\n';
        }
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

    //The window is gonna be maximized from the glfw hint above so the width and height are useless
    window = glfwCreateWindow(960, 540, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // if(glewInit() != GLEW_OK)
    //     std::cout << "Error!" << std::endl;
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_MULTISAMPLE));
    int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    {
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(),&width, &height);
        io.DisplaySize = ImVec2(width, height);
    }
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("Assets/font.ttf", 16.0f, &config);
    io.FontDefault = io.Fonts->Fonts.back();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    bool dockspaceOpen = true;

    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    glfwSetWindowUserPointer(window, this);
    // glfwSetWindowSizeCallback(window, OnResizeCallBack);
    // glfwSetWindowFocusCallback(window, OnWindowFocusCallBack);
}

void Application::Update()
{
    OnStart();
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        GLCall(glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        OnUpdate(DeltaTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    OnExit();
}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}

void Application::Exit()
{
    glfwWindowShouldClose(window);
}

bool Application::GetInput(int Key)
{
    return glfwGetKey(window, Key) == GLFW_PRESS;
}

bool Application::GetMouseInput(int MouseButton)
{
    return glfwGetMouseButton(window, MouseButton) == GLFW_PRESS;
}

bool Application::GetInputDown(int Key)
{
    if(glfwGetKey(window, Key) == GLFW_PRESS && Keys.find(Key) == Keys.end()){
        Keys[Key] = true;
        return true;
    }
    if(glfwGetKey(window, Key) == GLFW_RELEASE && Keys.find(Key) != Keys.end()){
        Keys.erase(Key);
        return false;
    }
    return false;
}

bool Application::GetMouseInputDown(int MouseButton)
{
    MouseButtons[MouseButton] = true;
    if(glfwGetMouseButton(window, MouseButton) == GLFW_RELEASE && MouseButtons.find(MouseButton) != MouseButtons.end()){
        MouseButtons.erase(MouseButton);
    }
    return glfwGetMouseButton(window, MouseButton) == GLFW_PRESS && MouseButtons.find(MouseButton) == MouseButtons.end();
}

void Application::OnWindowFocusCallBack(GLFWwindow *window, int focused)
{
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->OnWindowFocus(window, focused);
    }
}

void Application::OnResizeCallBack(GLFWwindow *window, int width, int height)
{
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->OnResize(window, width, height);
    }
}
