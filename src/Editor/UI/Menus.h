#pragma once
#include "ImGuiWindow.h"


void Toolbar();
void ViewMenu();
void HelpMenu();
void EditMenu();
void FileMenu();
void OnThemeChange();
void Load(std::string name);
inline static float SatAmount;
inline static std::map<ImGuiCol_, ImVec4> LoadedTheme;