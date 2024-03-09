#pragma once
#include "File.h"
#include <unordered_map>
#include "UI/DragAndDrop.hpp"
#include "Editor/UI/ImGuiWindow.h"

inline DragAndDrop<std::shared_ptr<File>> FileExplorerDrop;
inline DragAndDrop<std::shared_ptr<File>> AnimationDrop;
inline DragAndDrop<std::shared_ptr<ObjectRef>> HierachyDrop;

class FileExplorer : Window
{
    public:
        FileExplorer();
        void Display() override;
        const std::unordered_map<std::string, std::shared_ptr<File>>& GetFiles() {return m_Files;}
        void CopyAndOverwrite(const std::string& CopyFrom, const std::string& PasteTo);
        const TextureAtlas& GetAtlas() {return m_MinIconAtlas;}
        std::optional<std::string> InputFieldResult();
    private:
        TextureAtlas m_IconAtlas;
        TextureAtlas m_MinIconAtlas;
        std::unordered_map<std::string, std::shared_ptr<File>> m_Files;
        bool m_RightClicked;
        void RightClickPopUp(const std::filesystem::path& path);
        std::string m_SelectedFile;
        std::string m_NewFileName;
        bool m_SelectedCut;
        std::filesystem::path m_CopiedFilePath;
        bool Renaming = false;
        bool ShouldStopRenaming = true;
};