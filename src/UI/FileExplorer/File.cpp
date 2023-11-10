#include "File.h"
void File::RegisterFile(const std::string &extension, std::function<std::shared_ptr<File>()> creationFunction)
{
    File::FileCreationMap[extension] = creationFunction;
}

std::shared_ptr<File> File::CreateFile(const std::string &extension, std::string NewPath, std::string NewName)
{
    auto creationFunction = File::FileCreationMap.find(extension);
    if (creationFunction != File::FileCreationMap.end()) {
        std::shared_ptr NewFile = creationFunction->second();
        NewFile->Name = NewName;
        NewFile->Path = NewPath;
        NewFile->Extension = extension;
        NewFile->SetIconPos();
        return NewFile;
    }else if(std::filesystem::is_directory(Engine::Get().GetWindows().CurrentPath + NewPath))
    {
        std::shared_ptr NewFile = File::FileCreationMap["folder"]();
        NewFile->Name = NewName;
        NewFile->Path = NewPath;
        NewFile->Extension = "folder";
        NewFile->SetIconPos();
        return NewFile;
    }else{
        std::shared_ptr NewFile = File::FileCreationMap["default"]();
        NewFile->Name = NewName;
        NewFile->Path = NewPath;
        NewFile->Extension = extension;
        NewFile->SetIconPos();
        return NewFile;
    }
    return nullptr; // Unsupported file type
}

void File::RenderGUI(std::filesystem::directory_entry entry, ImVec2 Position, TextureAtlas& IconAtlas, const std::string& SelecteFile, bool& Renaming, bool& ShouldStopRenaming)
{
    ImGui::SetCursorPos(Position);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if(entry.path().string() == SelecteFile){
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
    }
    ImGui::BeginChild(entry.path().string().c_str(), ImVec2(m_IconSize.x + 25, m_IconSize.y + 50), true);

    glm::vec4 IconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(m_IconPos.x, m_IconPos.y), glm::vec2(m_IconSize.x, m_IconSize.y), IconAtlas.AtlasID.GetDimensions()); 
    ImGui::Image(reinterpret_cast<ImTextureID>(IconAtlas.AtlasID.GetID()), ImVec2(m_IconSize.x, m_IconSize.y), ImVec2(IconUVs.x, IconUVs.y), ImVec2(IconUVs.z, IconUVs.w));
    if(Renaming){
        ImGui::InputText("##RenamedFile",&Name);
        if(glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ENTER)){
            std::filesystem::rename(Engine::Get().GetWindows().CurrentPath + entry.path().filename().string(), Engine::Get().GetWindows().CurrentPath + Name);
            ShouldStopRenaming = true;
        }
    }else{
        ImGui::TextUnformatted(Name.c_str());
    }
    if(entry.path().string() == SelecteFile){
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

}

