#include "Engine.h"
#include "UI/FileExplorer/FileExplorer.h"
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
	// glViewport(0, 0, bufferWidth, bufferHeight);

    SapphireRenderer::LoadShader(const_cast<GLuint&>(Shapes::CircleShader.GetID()), "Shaders/Circle.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(Shapes::BasicShader.GetID()), "Shaders/Basic.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(Shapes::TextureShader.GetID()), "Shaders/Texture.glsl");

    #ifndef EXPORT
    m_Windows.Init(std::move(Path));
    m_Viewport.Init(&m_ActiveScene);
    m_PlayMode.Init(&m_ActiveScene);
    std::ifstream stream(m_Windows.MainPath + "/../ProjectSettings.json");
    nlohmann::json Data;
    stream >> Data;
    for (auto &&setting : Data.items())
    {
        m_Windows.SettingsVariables[setting.key()]->Load(setting.value());
    }
    stream.close();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    #endif

    #ifdef EXPORT
    m_Windows.MainPath = std::move(Path);
    m_PlayMode.Init(&m_ActiveScene);
    #endif
    glfwSetWindowSizeCallback(m_Window, ResizeIO);
    glfwSetWindowFocusCallback(m_Window, window_focus_callback);
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

        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        float currentTime = glfwGetTime();
        DeltaTime = currentTime - LastTime;
        LastTime = currentTime;

        m_Windows.DockSpace();
        m_Windows.Toolbar();
        m_Windows.ThemeMaker();
        m_Windows.TestWindow();

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
    {
        nlohmann::json ProjectSettingsJSON;
        std::ofstream stream(m_Windows.MainPath + "/../ProjectSettings.json");
        for (auto &&setting : m_Windows.SettingsVariables)
        {
            setting.second->Save(ProjectSettingsJSON);
        }
        stream << ProjectSettingsJSON.dump(2);
        stream.close();
    }
    {
        nlohmann::json UserPrefrencesJSON;
        std::ofstream stream("Assets/Preferences.json");
        for (auto &&setting : Windows::UserPreferences)
        {
            setting.second->Save(UserPrefrencesJSON);
        }
        stream << UserPrefrencesJSON.dump(2);
        stream.close();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void Engine::Export()
{
    if(!std::filesystem::exists(m_Windows.MainPath + "/../" + "Build")){
        std::filesystem::create_directories(m_Windows.MainPath + "/../" + "Build/Data");
    }
    //Copy pasting all the necessary dll files.
    FileExplorer::CopyAndOverwrite("glew32.dll", m_Windows.MainPath + "/../" + "Build/glew32.dll");
    FileExplorer::CopyAndOverwrite("glfw3.dll", m_Windows.MainPath + "/../" + "Build/glfw3.dll");
    FileExplorer::CopyAndOverwrite("lua54.dll", m_Windows.MainPath + "/../" + "Build/lua54.dll");
    if(!std::filesystem::exists(m_Windows.MainPath + "/../" + "Build/Shaders")){
        std::filesystem::copy("Shaders", m_Windows.MainPath + "/../" + "Build/Shaders");
    }
    /*
    I know that this solution feels kinda cheap but I'm not really interested in making the most optimized builds
    And all of that stuff, so I found a quick solution that works for the current state of the engine and allows me to
    focus on the other stuff of the engine.
    */
    FileExplorer::CopyAndOverwrite("Sapphire-Engine-Runtime.exe", m_Windows.MainPath + "/../" + "Build/Game.exe");
    for (auto &&object : m_ActiveScene.Objects)
    {
        for (auto &&component : object->GetComponents())
        {
            if(component->GetState() != nullptr){
                system(("C:/Users/bagge/Downloads/lua-5.4.2_Win64_bin/luac54.exe -o " + m_Windows.MainPath + "/../" + "/Build/Data/"+ component->GetFile() + " " + m_Windows.MainPath + component->GetFile()).c_str());
            }
        }
    }
    for(const auto &file : FileExplorer::GetFiles()){
        if(file.second->Name.erase(0, file.second->Name.size() - 5) != "scene") continue;
        FileExplorer::CopyAndOverwrite(m_Windows.MainPath + file.second->Path,m_Windows.MainPath + "/../" + "Build/Data/" + file.second->Path);
    }
    
}

Scene* Engine::GetActiveScene()
{
    return &m_ActiveScene;
}

const std::string &Engine::GetMainPath()
{
    return m_Windows.MainPath;
}
