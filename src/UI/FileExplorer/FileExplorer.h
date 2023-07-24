#pragma once
#include "File.h"
#include <unordered_map>
#include "UI/DragAndDrop.hpp"

inline DragAndDrop<std::shared_ptr<File>> FileExplorerDrop;
class FileExplorer
{
    public:
        static void Open(std::string path);
    private:
        inline static char NewFileName[256]; // The editing file name of the ImGui pop up
        inline static std::unordered_map<std::string, std::shared_ptr<File>> Files;
        inline static bool RightClicked;
        inline static void RightClickPopUp(std::filesystem::__cxx11::directory_entry entry);
        inline static std::string SelectedFile;
};