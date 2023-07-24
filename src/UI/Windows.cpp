#include "Windows.h"
#include "Graphics/Shapes.h"

void Windows::Init(std::string Path){

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
    ImGuiWindowFlags dockspaceFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    Windows::IconsAtlas.AtlasID = LoadTexture("Images/IconsAtlas.png");
    Windows::IconsAtlas.AtlasSize = glm::vec2(1095.0f,539.0f);
    Windows::IconsAtlas.IconSize = glm::vec2(579.0f,537.0f);
    ImGuiStyle& style = ImGui::GetStyle();

}
void Windows::DockSpace()
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("DockSpace", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void Windows::LogWindow()
{
    std::string windowName = "Logs";
    if (Utilities::Logs.size() <= 999) {
        windowName += " (" + std::to_string(Utilities::Logs.size()) + ")";
    } else {
        windowName += " (999+)";
    }
    windowName += "###LogWindow";
    ImGui::Begin(windowName.c_str());
    //Here I'm tryign to keep the button always visible even on scroll
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100.0f, ImGui::GetScrollY() + 20.0f));
    if (ImGui::Button("Clear"))
    {
        Utilities::Logs.clear();
    }
    ImGui::SetCursorPos(ImVec2(5, 30));
    for (size_t i = 0; i < Utilities::Logs.size(); i++)
    {
        switch (Utilities::Logs[i].second)
        {
        case Utilities::Info:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(Utilities::Logs[i].first.c_str());
            ImGui::PopStyleColor();
            break;
        case Utilities::Warning:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            ImGui::TextUnformatted(Utilities::Logs[i].first.c_str());
            ImGui::PopStyleColor();
            break;
        case Utilities::Error:
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::TextUnformatted(Utilities::Logs[i].first.c_str());
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