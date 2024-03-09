#pragma once
#include "ImGuiWindow.h"
#include "FileExplorer/FileExplorer.h"

void Toolbar(FileExplorer& fe);
void ViewMenu();
void HelpMenu();
void EditMenu();
void FileMenu(FileExplorer& fe);
void OnThemeChange();
void Load(std::string name);
inline static float SatAmount;
inline static std::map<ImGuiCol_, ImVec4> LoadedTheme;