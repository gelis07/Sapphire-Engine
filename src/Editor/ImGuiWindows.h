#pragma once
#include "Application/Application.h"
#include "Variables.h"
#include "json.hpp"

class File;
void LogWindow();
void Toolbar();
void FileMenu();
void Save(std::string name);
void Load(std::string name);
void EditMenu();
void FrameRate();
void HelpMenu();
void ViewMenu();
void OnThemeChange();
template <typename T>
void SaveProjectVariable(std::string&& path, std::string&& VariableName, T data);
inline static float SatAmount;
void PreferencesWindow();
void ProjectSettings();
void Settings();
void AnimationPreview();
void AnimationTimeline();
void OnFileDrop(std::shared_ptr<File> *File);
void RenderKeyFrames(size_t i, float WindowPosX);
inline static std::map<ImGuiCol_, ImVec4> LoadedTheme;