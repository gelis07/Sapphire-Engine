#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include <typeinfo>
#include <functional>
#include <typeindex>
#include "Engine/Views.h"
#include "windows.h" //This is NOT the Windows.h from this file its the windows for the WinExec() function
class File{
    public:
        inline static std::unordered_map<std::string, std::function<std::shared_ptr<File>()>> FileCreationMap;
        //Here we're registering that a file type exists for example ".lua", ".scene"
        static void RegisterFile(const std::string& extension, std::function<std::shared_ptr<File>()> creationFunction); 
        //Here we have a helper function that we can call from the file explorer to create a file based on the extension
        static std::shared_ptr<File> CreateFile(const std::string &extension, std::string NewPath, std::string NewName);
        void RenderGUI(std::filesystem::__cxx11::directory_entry entry, ImVec2 Position);
        virtual void OnClick(std::filesystem::__cxx11::directory_entry entry) {}
        virtual void OnRightClick(std::filesystem::__cxx11::directory_entry entry) {}
        virtual void OnDoubleClick(std::filesystem::__cxx11::directory_entry entry) {}

        std::string Path;
        std::string Name;
    protected:
        glm::vec2 IconPos;
};


class LuaFile : public File
{
    public:
        void OnClick(std::filesystem::__cxx11::directory_entry entry) 
        {
        }
        void OnDoubleClick(std::filesystem::__cxx11::directory_entry entry) override
        {
            WinExec((std::string("code ") + Windows::MainPath).c_str(), SW_HIDE);
        }
};

class SceneFile : public File
{
    public:
        void OnClick(std::filesystem::__cxx11::directory_entry entry) override
        {
        }
        void OnDoubleClick(std::filesystem::__cxx11::directory_entry entry) override
        {
            Viewport::CurrentScene->Load(entry.path().filename().string(), Viewport::window);
        }
};