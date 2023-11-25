#pragma once
#include "Application/Application.h"
#include "Variables.h"
#include "json.hpp"

void LogWindow();
void Toolbar();
void FileMenu();
void Save(std::string name);
void Load(std::string name);
void EditMenu();
void HelpMenu();
void ViewMenu();
void OnThemeChange();
template <typename T>
void SaveProjectVariable(std::string&& path, std::string&& VariableName, T data);
inline static float SatAmount;
void PreferencesWindow();
void ProjectSettings();
void Settings();
void AnimationTimeline();
inline static std::map<ImGuiCol_, ImVec4> LoadedTheme;