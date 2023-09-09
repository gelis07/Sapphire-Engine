#pragma once
#include "File.h"
#include <unordered_map>
#include "UI/DragAndDrop.hpp"

inline DragAndDrop<std::shared_ptr<File>> FileExplorerDrop;
class FileExplorer
{
    public:
        static void Open(std::string path);
        static void Init();
        static const std::unordered_map<std::string, std::shared_ptr<File>>& GetFiles() {return m_Files;}
        static void CopyAndOverwrite(std::string&& CopyFrom, std::string&& PasteTo);
        static const TextureAtlas& GetAtlas() {return m_IconAtlas;}
    private:
        inline static TextureAtlas m_IconAtlas;
        inline static std::unordered_map<std::string, std::shared_ptr<File>> m_Files;
        inline static bool m_RightClicked;
        inline static void RightClickPopUp(std::filesystem::path& path);
        inline static std::string m_SelectedFile;
        inline static std::string m_NewFileName;
        inline static bool m_SelectedCut;
        inline static std::filesystem::path m_CopiedFilePath;
        static void MouseInput(std::filesystem::path path);
};