#include "Application.h"
#include "Imgui/implot.h"
Application::Application(const glm::vec2& WindowDim, bool fullscreen,const std::string& Path)
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
    glfwWindowHint(GLFW_MAXIMIZED, fullscreen);

    //The window is gonna be maximized from the glfw hint above so the width and height are useless
    window = glfwCreateWindow(WindowDim.x, WindowDim.y, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(window);
    // if(glewInit() != GLEW_OK)
    //     std::cout << "Error!" << std::endl;
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_MULTISAMPLE));
    int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDoubleClickTime = 0.5;
    IMGUI_CHECKVERSION();
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    bool dockspaceOpen = true;

    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    ImPlot::CreateContext();
    // glfwSetWindowSizeCallback(window, OnResizeCallBack);
}
void Application::Update()
{
    OnStart();
    ImGuiIO& io = ImGui::GetIO();
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;
        OnUpdate(DeltaTime);

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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
    ImPlot::DestroyContext();
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
    if(glfwGetMouseButton(window, MouseButton) == GLFW_PRESS && MouseButtons.find(MouseButton) == MouseButtons.end()){
        MouseButtons[MouseButton] = true;
        return true;
    }
    if(glfwGetMouseButton(window, MouseButton) == GLFW_RELEASE && MouseButtons.find(MouseButton) != MouseButtons.end()){
        MouseButtons.erase(MouseButton);
        return false;
    }
    return false;
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
