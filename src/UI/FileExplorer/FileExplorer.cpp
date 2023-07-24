#include "FileExplorer.h"
#define SET_FILE(filetype) \
    static_cast<std::function<std::shared_ptr<File>()>>([]() -> std::shared_ptr<File> {return std::static_pointer_cast<File>(std::make_shared<filetype>());})

namespace fs = std::filesystem;



void Init(){
    File::RegisterFile(".lua", SET_FILE(LuaFile));
    File::RegisterFile(".scene", SET_FILE(SceneFile));
}

void FileExplorer::Open(std::string path)
{
    Init();
    ImGui::Begin("File Explorer");
    ImVec2 Position = ImVec2(0, 30);
    int rows = 1;
    for (const auto &entry : fs::directory_iterator(path))
    {
        std::string FileName = entry.path().filename().string();
        if(Files.find(FileName) == Files.end())
        {
            Files[FileName] = File::CreateFile(entry.path().extension().string(), entry.path().string(), FileName);
        }
        Files[FileName]->RenderGUI(entry, Position);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup(entry.path().filename().string().c_str());
            Files[FileName]->OnRightClick(entry);
            RightClicked = true;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            SelectedFile = entry.path().string(); // Getting the selected file so we can highlight it and modify it (delete it, rename, etc..)
            FileExplorerDrop.StartedDragging(Files[FileName]);
            Files[FileName]->OnClick(entry);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(0))
        {
            Files[FileName]->OnDoubleClick(entry);
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
        
        ImGui::InputText("New name", NewFileName, sizeof(NewFileName));
        if (ImGui::MenuItem("Rename!"))
        {
            std::string NewName = "Assets/" + std::string(NewFileName) + entry.path().extension().string();
            rename(entry.path().string().c_str(), NewName.c_str());
            ImGui::CloseCurrentPopup();
        }
        RightClicked = false;
    }
}