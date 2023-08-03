#pragma once
#include "Utilities.hpp"
#include "UI/Windows.h"
#include <typeinfo>
#include <functional>
#include <typeindex>
#include "Engine/Views/SceneEditor.h"
#include "windows.h" //This is NOT the Windows.h from this file its the windows for the WinExec() function

class File{
    public:
        inline static std::unordered_map<std::string, std::function<std::shared_ptr<File>()>> FileCreationMap;
        //Here we're registering that a file type exists for example ".lua", ".scene"
        static void RegisterFile(const std::string& extension, std::function<std::shared_ptr<File>()> creationFunction); 
        //Here we have a helper function that we can call from the file explorer to create a file based on the extension
        static std::shared_ptr<File> CreateFile(const std::string &extension, std::string NewPath, std::string NewName);
        void RenderGUI(std::filesystem::directory_entry entry, ImVec2 Position, TextureAtlas& IconAtlas);
        virtual void OnClick(std::filesystem::directory_entry entry) {}
        virtual void OnRightClick(std::filesystem::directory_entry entry) {}
        virtual void OnDoubleClick(std::filesystem::directory_entry entry) {}
        virtual void SetIconPos() {} // Indicates the position of the file's icon on the IconAtlas.png texture
        std::string Path;
        std::string Name;
    protected:
        glm::vec2 m_IconPos;
};


class LuaFile : public File
{
    public:
        void SetIconPos() override{ m_IconPos = glm::vec2(1,0);}
        void OnClick(std::filesystem::directory_entry entry) 
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            WinExec((std::string("code ") + GetMainPath()).c_str(), SW_HIDE);
            SapphireEngine::Log("Open script!", SapphireEngine::Info);
        }
};

class SceneFile : public File
{
    public:
        void SetIconPos() override{ m_IconPos = glm::vec2(0,0);}
        void OnClick(std::filesystem::directory_entry entry) override
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            GetActiveScene()->Load(entry.path().filename().string(), GetMainPath(),glfwGetCurrentContext());
        }
};