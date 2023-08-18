#include "FileExplorer.h"
#define SET_FILE(filetype) \
    static_cast<std::function<std::shared_ptr<File>()>>([]() -> std::shared_ptr<File> {return std::static_pointer_cast<File>(std::make_shared<filetype>());})

namespace fs = std::filesystem;



void FileExplorer::Init(){
    File::RegisterFile("default", SET_FILE(Default));
    File::RegisterFile(".lua", SET_FILE(LuaFile));
    File::RegisterFile(".scene", SET_FILE(SceneFile));
    m_IconAtlas.AtlasID = LoadTexture("Assets/IconsAtlas.png");
    m_IconAtlas.AtlasSize = glm::vec2(2304,512);
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
            m_Files[FileName] = File::CreateFile(entry.path().extension().string(), entry.path().string().erase(0,Engine::Get().GetMainPath().size() - 1), FileName);
        }
        m_Files[FileName]->RenderGUI(entry, Position, m_IconAtlas, m_SelectedFile);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
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
    MouseInput(m_SelectedFile);
    ImGui::End();
}

void FileExplorer::RightClickPopUp(std::filesystem::path& path)
{
    if (ImGui::BeginPopup("Context Menu"))
    {

        if (ImGui::MenuItem("Create lua script"))
        {
            std::ofstream stream(Engine::Get().GetMainPath() + "NewLuaScript.lua", std::ofstream::trunc);
            stream << R"(local SapphireEngine = require("SapphireEngine")

function OnStart()
    SapphireEngine.Log("Hello World!")
end

function OnUpdate()

end

function OnCollision()

end)";
            stream.close();
        }
        // if (ImGui::MenuItem("Copy"))
        // {
        //     m_CopiedFilePath = &path;
        //     m_SelectedCut = false;
        // }
        // if (ImGui::MenuItem("Cut"))
        // {
        //     m_CopiedFilePath = &path;
        //     m_SelectedCut = true;
        // }
        // if (ImGui::MenuItem("Paste"))
        // {
        //     fs::copy_file(*m_CopiedFilePath, "C:/Gelis/test.scene", fs::copy_options::overwrite_existing);
        //     if(m_SelectedCut){
        //         fs::remove(*m_CopiedFilePath);
        //     }
        // }
        ImGui::EndPopup();
    }
}


void FileExplorer::MouseInput(std::filesystem::path path)
{
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("Context Menu");
    }
    RightClickPopUp(path);
}
