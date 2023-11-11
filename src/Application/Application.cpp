#include "Application.h"

Application::Application()
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

    //The window is gonna be maximized from the glfw hint above so the width and height are useless
    window = glfwCreateWindow(960, 540, "Sapphire Engine", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if(glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, OnResizeCallBack);
    glfwSetWindowFocusCallback(window, OnWindowFocusCallBack);
    OnStart();
}

void Application::Update()
{
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
}

Application::~Application()
{
    OnExit();
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
    Keys[Key] = true;
    if(glfwGetKey(window, Key) == GLFW_RELEASE && Keys.find(Key) != Keys.end()){
        Keys.erase(Key);
    }
    return glfwGetKey(window, Key) == GLFW_PRESS && Keys.find(Key) == Keys.end();
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
