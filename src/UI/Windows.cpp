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
    InitWindow("ThemeMaker", false);
    Load("purple");
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
        ThemeMenu();
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
        if(ImGui::Button("Export")){
            Engine::Get().Export();
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
ImVec4 Saturate(ImVec4 StartingCol,float amount){
    return ImVec4(StartingCol.x + amount / 255.0f, StartingCol.y + amount / 255.0f, StartingCol.z + amount / 255.0f, 1);
}
void Windows::OnThemeChange()
{
    ImGuiStyle& style = ImGui::GetStyle();


    LoadedTheme[ImGuiCol_TabUnfocused] = LoadedTheme[ImGuiCol_Tab];
    LoadedTheme[ImGuiCol_TabUnfocusedActive] = LoadedTheme[ImGuiCol_Tab];
    LoadedTheme[ImGuiCol_TabActive] = Saturate(LoadedTheme[ImGuiCol_Tab], SatAmount);
    LoadedTheme[ImGuiCol_TabHovered] = Saturate(LoadedTheme[ImGuiCol_Tab], 20);

    LoadedTheme[ImGuiCol_TitleBgActive] = LoadedTheme[ImGuiCol_TitleBg], SatAmount;

    LoadedTheme[ImGuiCol_Button] = LoadedTheme[ImGuiCol_FrameBg];
    LoadedTheme[ImGuiCol_ButtonActive] = Saturate(LoadedTheme[ImGuiCol_FrameBg], SatAmount);
    LoadedTheme[ImGuiCol_ButtonHovered] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);
    LoadedTheme[ImGuiCol_FrameBgActive] = Saturate(LoadedTheme[ImGuiCol_FrameBg], SatAmount);
    LoadedTheme[ImGuiCol_FrameBgHovered] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);
    LoadedTheme[ImGuiCol_ChildBg] = Saturate(LoadedTheme[ImGuiCol_WindowBg], SatAmount);

    for(auto const& x : LoadedTheme)
    {
        style.Colors[x.first] = x.second;
    }

    style.FrameRounding = 4;
    style.PopupRounding = 2;
    style.GrabRounding = 1;
    style.TabRounding = 6;
    style.TabRounding = 1;
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

void Windows::ThemeMenu()
{
    if(ImGui::BeginMenu("Themes")){
        for(const auto &entry : std::filesystem::directory_iterator("Themes/")){
            std::string name = entry.path().filename().string();
            name = name.erase(name.size() - 5, name.size());
            if(ImGui::Button(name.c_str()))
            {
                Load(name);
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


void Windows::Save(std::string name){
    std::ofstream stream("Themes/" + name +".json", std::ofstream::trunc);
    nlohmann::json Theme;
    Theme["ImGuiCol_WindowBg"] = {LoadedTheme[ImGuiCol_WindowBg].x, LoadedTheme[ImGuiCol_WindowBg].y, LoadedTheme[ImGuiCol_WindowBg].z,LoadedTheme[ImGuiCol_WindowBg].w};
    Theme["ImGuiCol_Tab"] = {LoadedTheme[ImGuiCol_Tab].x, LoadedTheme[ImGuiCol_Tab].y, LoadedTheme[ImGuiCol_Tab].z,LoadedTheme[ImGuiCol_Tab].w};
    Theme["ImGuiCol_TitleBg"] = {LoadedTheme[ImGuiCol_TitleBg].x, LoadedTheme[ImGuiCol_TitleBg].y, LoadedTheme[ImGuiCol_TitleBg].z,LoadedTheme[ImGuiCol_TitleBg].w};
    Theme["ImGuiCol_FrameBg"] = {LoadedTheme[ImGuiCol_FrameBg].x, LoadedTheme[ImGuiCol_FrameBg].y, LoadedTheme[ImGuiCol_FrameBg].z,LoadedTheme[ImGuiCol_FrameBg].w};
    Theme["SatAmount"] = SatAmount;
    stream << Theme.dump(2);
    stream.close();
}

void Windows::Load(std::string name){
    std::ifstream stream("Themes/" + name + ".json");
    nlohmann::json Theme;
    stream >> Theme;
    stream.close();
    
    LoadedTheme[ImGuiCol_WindowBg] = ImVec4(Theme["ImGuiCol_WindowBg"][0], Theme["ImGuiCol_WindowBg"][1], Theme["ImGuiCol_WindowBg"][2], Theme["ImGuiCol_WindowBg"][3]);
    LoadedTheme[ImGuiCol_Tab] = ImVec4(Theme["ImGuiCol_Tab"][0], Theme["ImGuiCol_Tab"][1], Theme["ImGuiCol_Tab"][2], Theme["ImGuiCol_Tab"][3]);
    LoadedTheme[ImGuiCol_TitleBg] = ImVec4(Theme["ImGuiCol_TitleBg"][0], Theme["ImGuiCol_TitleBg"][1], Theme["ImGuiCol_TitleBg"][2], Theme["ImGuiCol_TitleBg"][3]);
    LoadedTheme[ImGuiCol_FrameBg] = ImVec4(Theme["ImGuiCol_FrameBg"][0], Theme["ImGuiCol_FrameBg"][1], Theme["ImGuiCol_FrameBg"][2], Theme["ImGuiCol_FrameBg"][3]);
    SatAmount = Theme["SatAmount"];
    OnThemeChange();
}
std::string ThemeName;
void Windows::ThemeMaker()
{
    if(!(*GetWindowState("ThemeMaker"))) return;
    ImGui::Begin("ThemeMaker", GetWindowState("ThemeMaker"));

    ImGui::ColorEdit4("ImGuiCol_WindowBg", (float*)&(LoadedTheme[ImGuiCol_WindowBg]));
    ImGui::ColorEdit4("ImGuiCol_Tab", (float*)&(LoadedTheme[ImGuiCol_Tab]));
    ImGui::ColorEdit4("ImGuiCol_TitleBg", (float*)&(LoadedTheme[ImGuiCol_TitleBg]));
    ImGui::ColorEdit4("ImGuiCol_FrameBg", (float*)&(LoadedTheme[ImGuiCol_FrameBg]));
    ImGui::SliderFloat("Saturate", &SatAmount, -255.0f, 255.0f);
    if(ImGui::Button("Save")){
        ImGui::OpenPopup("Save");
    }
    if(ImGui::Button("Load")){
        ImGui::OpenPopup("Load");
    }

    if(ImGui::BeginPopup("Save")){
        ImGui::InputText("Theme Name", &ThemeName);
        if(ImGui::Button("Save")){
            Save(ThemeName);
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}