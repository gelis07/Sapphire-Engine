#include "Windows.h"

void Windows::Init(std::string&& Path){

    IMGUI_CHECKVERSION();
    DefaultContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    IO = &io;
    IMGUI_CHECKVERSION();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(600, 400);
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("Assets/font.ttf", 16.0f, &config);
    io.FontDefault = io.Fonts->Fonts.back();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    bool dockspaceOpen = true;
    this->MainPath = Path;
    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiStyle& style = ImGui::GetStyle();
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
    std::string windowName = "Logs";
    if (SapphireEngine::Logs.size() <= 999) {
        windowName += " (" + std::to_string(SapphireEngine::Logs.size()) + ")";
    } else {
        windowName += " (999+)";
    }
    windowName += "###LogWindow";
    ImGui::Begin(windowName.c_str());
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

void Windows::Toolbar()
{
    if (ImGui::BeginMainMenuBar())
    {
//        FileMenu();
//        if(Get->SceneFile == ""){
//            ImGui::Text("No scene loaded");
//        }else{
//            ImGui::Text(std::string("Viewing scene: " + CurrentScene->SceneFile).c_str());
//        }
        ImGui::EndMainMenuBar();
    }
}
//void Windows::FileMenu() const {
//    if (ImGui::BeginMenu("File"))
//    {
//        if(ImGui::Selectable("Save"))
//        {
//            ImGui::OpenPopup("Save Menu");
//        }
//        if (ImGui::BeginPopup("Save Menu"))
//        {
//            ImGui::InputText("Scene Name", Name);
//            if (ImGui::MenuItem("Save"))
//            {
//                CurrentScene->Save(std::string(Name) + ".scene");
//            }
//
//            ImGui::EndPopup();
//        }
//        if(ImGui::Selectable("Load"))
//        {
//            ImGui::OpenPopup("Load Menu");
//        }
//        if (ImGui::BeginPopup("Load Menu"))
//        {
//            ImGui::InputText("Scene Name", Name, sizeof(Name));
//            if (ImGui::MenuItem("Load"))
//            {
//                CurrentScene->Load(std::string(Name), window);
//            }
//
//            ImGui::EndPopup();
//        }
//
//        ImGui::EndMenu();
//    }
//}
//
//void Windows::File() const{if (ImGui::BeginMenu("File"))
//        {
//            if(ImGui::Selectable("Save"))
//            {
//                ImGui::OpenPopup("Save Menu");
//            }
//            if (ImGui::BeginPopup("Save Menu"))
//            {
//                ImGui::InputText("Scene Name", Name, sizeof(Name), ImGuiInputTextFlags_CharsNoBlank);
//                if (ImGui::MenuItem("Save"))
//                {
//                    CurrentScene->Save(std::string(Name) + ".scene");
//                }
//
//                ImGui::EndPopup();
//            }
//            if(ImGui::Selectable("Load"))
//            {
//                ImGui::OpenPopup("Load Menu");
//            }
//            if (ImGui::BeginPopup("Load Menu"))
//            {
//                ImGui::InputText("Scene Name", Name, sizeof(Name));
//                if (ImGui::MenuItem("Load"))
//                {
//                    CurrentScene->Load(std::string(Name), window);
//                }
//
//                ImGui::EndPopup();
//            }
//
//            ImGui::EndMenu();
//        }}
