#include "ImGuiWindow.h"

Window::Window(const std::string& name, bool Default) : Name(name)
{
    Windows.push_back(this);
    Active = Default;
}

void Window::Update()
{
    ImGui::Begin(Name.c_str(), &Active);
    Display();
    ImGui::End();
}

WindowFBO::WindowFBO(const std::string &name) : fbo(), Window(name)
{
    fbo.Init();
}
void WindowFBO::Display()
{  
    width = ImGui::GetContentRegionAvail().x;
    height = ImGui::GetContentRegionAvail().y;
    fbo.Bind();
    if(LastSize.x != width || LastSize.y != height){
        fbo.RescaleFrameBuffer(width, height);
        GLCall(glViewport(0, 0, width, height));
        LastSize = ImVec2(width, height);
    }
    Draw();
    fbo.Blit(width, height);
    fbo.Unbind();


    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID *>(fbo.RendFrameBuffer.GetTexture().GetID()),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + width, pos.y + height),
        ImVec2(0, 1),
        ImVec2(1, 0));
    
    Extras();
}

void LogWindow::Display()
{
    Name = "Logs";
    if (SapphireEngine::Logs.Size() <= 999) {
        Name += " (" + std::to_string(SapphireEngine::Logs.Size()) + ")";
    } else {
        Name += " (999+)";
    }
    Name += "###LogWindow";
    ImGui::TextUnformatted(SapphireEngine::Logs.GetContent().str().c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    //Here I'm tryign to keep the button always visible even on scroll
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100.0f, ImGui::GetScrollY() + 20.0f));
    //I need to use a child window because otherwise ImGui will render the other child windows (the logs above) and that will  
    //cause the button to be clickable only under the logs and makes it hard to click.
    if (ImGui::Button("Clear"))
    {
        SapphireEngine::Logs.Clear();
    }
}
