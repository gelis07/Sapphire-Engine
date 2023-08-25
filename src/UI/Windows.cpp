#include "Windows.h"
#include "Engine/Engine.h"


void Windows::Init(std::string&& Path){

    IMGUI_CHECKVERSION();
    DefaultContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    IO = &io;
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
    this->MainPath = Path;
    this->CurrentPath = Path;
    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiStyle& style = ImGui::GetStyle();

    InitWindow("Preferences", false);
    InitWindow("Settings", false);
    InitWindow("Project Settings", false);

    InitWindow("Inspector");
    InitWindow("Hierachy");
    InitWindow("Logs");
}
void Windows::DockSpace()
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void Windows::LogWindow()
{
    if(!(*GetWindowState("Logs"))) return;
    std::string windowName = "Logs";
    if (SapphireEngine::Logs.size() <= 999) {
        windowName += " (" + std::to_string(SapphireEngine::Logs.size()) + ")";
    } else {
        windowName += " (999+)";
    }
    windowName += "###LogWindow";
    ImGui::Begin(windowName.c_str(), GetWindowState("Logs"));
    //Here I'm tryign to keep the button always visible even on scroll
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100.0f, ImGui::GetScrollY() + 20.0f));
    if (ImGui::Button("Clear"))
    {
        SapphireEngine::Logs.clear();
    }
    ImGui::SetCursorPos(ImVec2(5, 30));
    for (size_t i = 0; i < SapphireEngine::Logs.size(); i++)
    {
        switch (SapphireEngine::Logs[i].second)
        {
            case SapphireEngine::Info:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::TextUnformatted(SapphireEngine::Logs[i].first.c_str());
                ImGui::PopStyleColor();
                break;
            case SapphireEngine::Warning:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(SapphireEngine::Logs[i].first.c_str());
                ImGui::PopStyleColor();
                break;
            case SapphireEngine::Error:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(SapphireEngine::Logs[i].first.c_str());
                ImGui::PopStyleColor();
                break;
            default:
                break;
        }
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::End();
}

void Windows::InitWindow(std::string &&WindowName, bool state)
{
    WindowStates[WindowName] = state;
}

void Windows::SetWindowState(std::string &&WindowName, bool state)
{
    if(WindowStates.find(WindowName) != WindowStates.end()){
        WindowStates[WindowName] = state;
    }else{
        SapphireEngine::Log(WindowName + " window doesn't exist", SapphireEngine::Error);
    }
}

bool* Windows::GetWindowState(std::string &&WindowName)
{
    if(WindowStates.find(WindowName) != WindowStates.end()){
        return &WindowStates[WindowName];
    }else{
        SapphireEngine::Log(WindowName + " window doesn't exist", SapphireEngine::Error);
    }
    return nullptr;
}

std::string Name;
void Windows::Toolbar()
{
    if (ImGui::BeginMainMenuBar())
    {
        FileMenu();
        EditMenu(); //& Got to save these features on a JSON file!
        ViewMenu();
        HelpMenu();
        if(Engine::Get().GetActiveScene()->SceneFile == ""){
            ImGui::Text("No scene loaded");
        }else{
            ImGui::Text(std::string("Viewing scene: " + Engine::Get().GetActiveScene()->SceneFile).c_str());
        }
        ImGui::EndMainMenuBar();
    }

}
void Windows::FileMenu(){
    if (ImGui::BeginMenu("File"))
    {
        if(ImGui::Button("Save"))
        {
            if(Engine::Get().GetActiveScene()->SceneFile != ""){
                Engine::Get().GetActiveScene()->Save(Engine::Get().GetActiveScene()->SceneFile);
            }else{
                ImGui::OpenPopup("FileSaveMenu");
            }
        }
        if(ImGui::Button("Save As"))
        {
            ImGui::OpenPopup("FileSaveMenu");
        }
        if (ImGui::BeginPopup("FileSaveMenu"))
        {
            ImGui::InputText("Scene Name", &Name);
            if (ImGui::MenuItem("Save"))
            {
                Engine::Get().GetActiveScene()->Save(std::string(Name) + ".scene");
            }

            ImGui::EndPopup();
        }
        ImGui::EndMenu();
    }
}

void Windows::EditMenu()
{
    if (ImGui::BeginMenu("Edit"))
    {
        if(ImGui::Button("Settings"))
        {
            SetWindowState("Settings", true);
        }
        if(ImGui::Button("Preferences"))
        {
            SetWindowState("Preferences", true);
        }
        if(ImGui::Button("Project Settings"))
        {
            SetWindowState("Project Settings", true);
        }
        ImGui::EndMenu();
    }

    PreferencesWindow();
    ProjectSettings();
    Settings();
}

void Windows::HelpMenu()
{
    if (ImGui::BeginMenu("Help"))
    {
        if(ImGui::Button("Documentation"))
        {

        }
        if(ImGui::Button("GitHub Repository"))
        {
            system("START https://github.com/gelis07/Sapphire-Engine");
        }
        ImGui::EndMenu();
    }
}

void Windows::ViewMenu()
{
    if(ImGui::BeginMenu("View")){
        for (auto &&Window : WindowStates)
        {
            if(ImGui::Button(Window.first.c_str()))
            {
                Window.second = !Window.second;
            }
        }
        ImGui::EndMenu();
    }

    
}

void Windows::PreferencesWindow()
{
    if(!(*GetWindowState("Preferences"))) return;
    ImGui::Begin("Preferences", &WindowStates["Preferences"]);

    
    ImGui::Text("Options coming soon.");

    ImGui::End();
}

void Windows::ProjectSettings()
{
    if(!(*GetWindowState("Project Settings"))) return;
    ImGui::Begin("Project Settings", &WindowStates["Project Settings"]);

    ImGui::DragFloat("gravitational acceleration", &PhysicsEngine::g);
    ImGui::Text("More options coming soon.");

    ImGui::End();
}

void Windows::Settings()
{
    if(!(*GetWindowState("Settings"))) return;
    ImGui::Begin("Settings", &WindowStates["Settings"]);

    
    ImGui::Text("Options coming soon.");


    ImGui::End();
}
