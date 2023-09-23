#pragma once
#include <typeinfo>
#include <functional>
#include <typeindex>
#include "Engine/Engine.h"

class File{
    public:
        inline static std::unordered_map<std::string, std::function<std::shared_ptr<File>()>> FileCreationMap;
        //Here we're registering that a file type exists for example ".lua", ".scene"
        static void RegisterFile(const std::string& extension, std::function<std::shared_ptr<File>()> creationFunction); 
        //Here we have a helper function that we can call from the file explorer to create a file based on the extension
        static std::shared_ptr<File> CreateFile(const std::string &extension, std::string NewPath, std::string NewName);
        void RenderGUI(std::filesystem::directory_entry entry, ImVec2 Position, TextureAtlas& IconAtlas, const std::string& SelecteFile, bool& Renaming, bool& ShouldStopRenaming);
        virtual void OnClick(std::filesystem::directory_entry entry) {}
        virtual void OnRightClick(std::filesystem::directory_entry entry) {}
        virtual void OnDoubleClick(std::filesystem::directory_entry entry) {}
        virtual void SetIconPos() {} // Indicates the position of the file's icon on the IconAtlas.png texture
        std::string Path;
        std::string Name;
    protected:
        glm::vec2 m_IconPos = glm::vec2(0);
        glm::vec2 m_IconSize = glm::vec2(0);
};

//For the "unsupported" files.
class Default : public File
{
    public:
        void SetIconPos() override{ 
            m_IconPos = glm::vec2(512*2,0); 
            m_IconSize = glm::vec2(512,512); 
        }
        void OnClick(std::filesystem::directory_entry entry) 
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
        }
};
class Folder : public File
{
    public:
        void SetIconPos() override{ 
            m_IconPos = glm::vec2(512*5,0); 
            m_IconSize = glm::vec2(512,512); 
        }
        void OnClick(std::filesystem::directory_entry entry) 
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            Engine::Get().GetWindows().CurrentPath = entry.path().string();
        }
};

class LuaFile : public File
{
    public:
        void SetIconPos() override{ 
            m_IconPos = glm::vec2(512*3,0); 
            m_IconSize = glm::vec2(512,512); 
        }
        void OnClick(std::filesystem::directory_entry entry) 
        {
            
        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            system(("code " + entry.path().string()).c_str()); // Open the script in visual studio code
        }
};

class SceneFile : public File
{
    public:
        void SetIconPos() override{
            m_IconPos = glm::vec2(0,0);
            m_IconSize = glm::vec2(512,512); 
        }
        void OnClick(std::filesystem::directory_entry entry) override
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            Engine::Get().GetActiveScene()->Load(entry.path().filename().string());
        }
};