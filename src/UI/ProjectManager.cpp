#include "ProjectManager.h"
#include "json.hpp"


ProjectManager::ProjectManager()
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    window = glfwCreateWindow(600, 400, "Sapphire Engine Manager", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    NewIo = &io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(600, 400);
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("Assets/font.ttf", 16.0f, &config);
    io.FontDefault = io.Fonts->Fonts.back();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
}

char ProjectPath[256];
char ProjectName[256];
int ProjectsSize = 0;
bool CloseApp = true; // If the user directly closes the app from the X button GLFW should be terminated. Thats the use of this boolean
nlohmann::json Projects;

void ProjectManager::SaveJson(std::string Name){
    std::ofstream stream("Projects.sp", std::ios::trunc);
    nlohmann::json Settings;
    Settings["Windows::MainPath"] = Windows::MainPath + "/" + Name;
    Settings["Name"] = Name;
    ProjectsSize++;
    Projects["Project " + std::to_string(ProjectsSize)] = Settings;
    stream << Projects.dump(2);
    stream.close();
}

std::string ProjectManager::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        int x, y;
        glfwGetWindowSize(window, &x, &y);
        ImGui::GetIO().DisplaySize = ImVec2(x, y);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Fullscreen Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 20 - 30, 30));
        if(ImGui::Button("Create", ImVec2(30,30))){
            ImGui::OpenPopup("Create Project Popup");
        }
        if(ImGui::BeginPopup("Create Project Popup")){
            std::string Name(ProjectName);
            ImGui::InputText("Projet Name", ProjectName, sizeof(ProjectPath));
            ImGui::InputText("Projet Path", ProjectPath, sizeof(ProjectPath));
            if(ImGui::Button("Create Project")){
                Windows::MainPath = std::string(ProjectPath);
                std::replace( Name.begin(), Name.end(), ' ', '_');
                fs::create_directories(Windows::MainPath + "/" + Name + "/Assets/");
                SaveJson(Name);
            }
            ImGui::EndPopup();
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 20 - 30 - 30, 30));
        if(ImGui::Button("Open", ImVec2(30,30))){
            ImGui::OpenPopup("Open Project Popup");
        }
        if(ImGui::BeginPopupContextWindow("Open Project Popup")){
            ImGui::SetNextWindowFocus();
            ImGui::InputText("Projet Path##3", ProjectPath, sizeof(char) * 256);
            std::string Name;
            if(ImGui::Button("Open")){
                Windows::MainPath = std::string(ProjectPath);
                fs::path path = Windows::MainPath;
                Name = path.root_name().string();
                std::replace( Name.begin(), Name.end(), ' ', '_');
                fs::create_directories(Windows::MainPath + "/" + Name + "/Assets/");
                SaveJson(Name);
            }

            ImGui::EndPopup();

        }


        std::ifstream stream("Projects.sp");
        nlohmann::json Data;
        stream >> Data;
        for (auto& Project : Data.items())
        {
            if(ImGui::Button(Project.value()["Name"].get<std::string>().c_str()))
            {
                Windows::MainPath = Project.value()["MainPath"].get<std::string>() + "/Assets/";
                CloseApp = false;
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        
        ImGui::End();
        GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(window));
        GLCall(glfwPollEvents());
    }
    glfwDestroyWindow(window);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if(CloseApp){
        glfwTerminate();
        std::exit(0);
    }

    return Windows::MainPath;
}
