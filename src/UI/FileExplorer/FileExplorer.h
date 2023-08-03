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
    private:
        inline static TextureAtlas m_IconAtlas;
        inline static std::unordered_map<std::string, std::shared_ptr<File>> m_Files;
        inline static bool m_RightClicked;
        inline static void RightClickPopUp(std::filesystem::__cxx11::directory_entry entry);
        inline static std::string m_SelectedFile;
        inline static std::string m_NewFileName;
};