#include "FileExplorer.h"
#define SET_FILE(filetype) \
    static_cast<std::function<std::shared_ptr<File>()>>([]() -> std::shared_ptr<File> {return std::static_pointer_cast<File>(std::make_shared<filetype>());})

namespace fs = std::filesystem;



void FileExplorer::Init(){
    File::RegisterFile(".lua", SET_FILE(LuaFile));
    File::RegisterFile(".scene", SET_FILE(SceneFile));
    m_IconAtlas.AtlasID = LoadTexture("Assets/IconsAtlas.png");
    m_IconAtlas.AtlasSize = glm::vec2(1095.0f,539.0f);
    m_IconAtlas.IconSize = glm::vec2(579.0f,537.0f);
}

void FileExplorer::Open(std::string path)
{
    ImGui::Begin("File Explorer");
    ImVec2 Position = ImVec2(0, 30);
    int rows = 1;
    for (const auto &entry : fs::directory_iterator(path))
    {
        std::string FileName = entry.path().filename().string();
        if(m_Files.find(FileName) == m_Files.end())
        {
            m_Files[FileName] = File::CreateFile(entry.path().extension().string(), entry.path().string(), FileName);
        }
        m_Files[FileName]->RenderGUI(entry, Position, m_IconAtlas);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup(entry.path().filename().string().c_str());
            m_Files[FileName]->OnRightClick(entry);
            m_RightClicked = true;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_SelectedFile = entry.path().string(); // Getting the selected file so we can highlight it and modify it (delete it, rename, etc..)
            FileExplorerDrop.StartedDragging(m_Files[FileName]);
            m_Files[FileName]->OnClick(entry);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(0))
        {
            m_Files[FileName]->OnDoubleClick(entry);
        }
        //Adjusting file's position so everything is spaced out.
        Position.x += ImGui::GetItemRectSize().x + 20.0f;
        if (ImGui::GetWindowSize().x < Position.x)
        {
            Position = ImVec2(0, 30 + ImGui::GetItemRectSize().y + 20.0f * rows);
            rows++;
        }
    }
    ImGui::End();
}

void FileExplorer::RightClickPopUp(std::filesystem::__cxx11::directory_entry entry)
{
    if (ImGui::BeginPopupContextWindow(entry.path().filename().string().c_str()))
    {
        
        ImGui::InputText("New name", &m_NewFileName);
        if (ImGui::MenuItem("Rename!"))
        {
            std::string NewName = "Assets/" + std::string(m_NewFileName) + entry.path().extension().string();
            rename(entry.path().string().c_str(), NewName.c_str());
            ImGui::CloseCurrentPopup();
        }
        m_RightClicked = false;
    }
}