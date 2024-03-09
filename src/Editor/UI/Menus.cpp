#include "Menus.h"
#include "Engine/Engine.h"
#include "Editor/Editor.h"

std::string Name;
void Toolbar(FileExplorer& fe)
{
    if (ImGui::BeginMainMenuBar())
    {
        FileMenu(fe);
        EditMenu();
        ViewMenu();
        HelpMenu();
        if(Engine::GetActiveScene().SceneFile == ""){
            ImGui::Text("No scene loaded");
        }else{
            ImGui::Text(std::string("Viewing scene: " + Engine::GetActiveScene().SceneFile).c_str());
        }
        ImGui::EndMainMenuBar();
    }
}
void FileMenu(FileExplorer& fe){
    if (ImGui::BeginMenu("File"))
    {
        if(ImGui::Button("Save"))
        {
            if(Engine::GetActiveScene().SceneFile != ""){
                Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);
            }else{
                ImGui::OpenPopup("FileSaveMenu");
            }
        }
        if(ImGui::Button("Save As"))
        {
            ImGui::OpenPopup("FileSaveMenu");
        }
        if(ImGui::Button("Export")){
            Editor::Export(fe);
        }
        if (ImGui::BeginPopup("FileSaveMenu"))
        {
            ImGui::InputText("Scene Name", &Name);
            if (ImGui::MenuItem("Save"))
            {
                Engine::GetActiveScene().Save(std::string(Name) + ".scene");
            }

            ImGui::EndPopup();
        }
        ImGui::EndMenu();
    }
}
void EditMenu()
{
    // if (ImGui::BeginMenu("Edit"))
    // {
    //     if(ImGui::Button("Settings"))
    //     {
    //         Editor::SetWindowState("Settings", true);
    //     }
    //     if(ImGui::Button("Preferences"))
    //     {
    //         Editor::SetWindowState("Preferences", true);
    //     }
    //     if(ImGui::Button("Project Settings"))
    //     {
    //         Editor::SetWindowState("Project Settings", true);
    //     }
    //     if(ImGui::Button("Performance"))
    //     {
    //         Editor::SetWindowState("Performance", true);
    //     }
    //     ImGui::EndMenu();
    // }
    // // PerformanceWindow();
    // PreferencesWindow();
    // ProjectSettings();
    // Settings();
}
void HelpMenu()
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
void ViewMenu()
{
    if(ImGui::BeginMenu("View")){
        for (auto &&Window : Window::Windows)
        {
            if(ImGui::Button(Window->GetName().c_str()))
            {
                Window->SetState(!Window->GetState());
            }
        }
        ImGui::EndMenu();
    }
}

void Load(std::string name){
    std::ifstream stream("Themes/" + name + ".json");
    nlohmann::ordered_json Theme;
    stream >> Theme;
    stream.close();
    
    LoadedTheme[ImGuiCol_WindowBg] = ImVec4(Theme["ImGuiCol_WindowBg"][0], Theme["ImGuiCol_WindowBg"][1], Theme["ImGuiCol_WindowBg"][2], Theme["ImGuiCol_WindowBg"][3]);
    LoadedTheme[ImGuiCol_Tab] = ImVec4(Theme["ImGuiCol_Tab"][0], Theme["ImGuiCol_Tab"][1], Theme["ImGuiCol_Tab"][2], Theme["ImGuiCol_Tab"][3]);
    LoadedTheme[ImGuiCol_TitleBg] = ImVec4(Theme["ImGuiCol_TitleBg"][0], Theme["ImGuiCol_TitleBg"][1], Theme["ImGuiCol_TitleBg"][2], Theme["ImGuiCol_TitleBg"][3]);
    LoadedTheme[ImGuiCol_FrameBg] = ImVec4(Theme["ImGuiCol_FrameBg"][0], Theme["ImGuiCol_FrameBg"][1], Theme["ImGuiCol_FrameBg"][2], Theme["ImGuiCol_FrameBg"][3]);
    SatAmount = Theme["SatAmount"];
    OnThemeChange();
}
ImVec4 Saturate(ImVec4 StartingCol,float amount){
    return ImVec4(StartingCol.x + amount / 255.0f, StartingCol.y + amount / 255.0f, StartingCol.z + amount / 255.0f, 1);
}
void OnThemeChange()
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
    LoadedTheme[ImGuiCol_TextSelectedBg] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);

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