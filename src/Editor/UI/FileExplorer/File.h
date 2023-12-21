#pragma once
#include <typeinfo>
#include <functional>
#include <typeindex>
#include "Editor.h"

constexpr int ICON_SIZE = 128;

class File{
    public:
        inline static std::unordered_map<std::string, std::function<std::shared_ptr<File>()>> FileCreationMap;
        //Here we're registering that a file type exists for example ".lua", ".scene"
        static void RegisterFile(const std::string& extension, std::function<std::shared_ptr<File>()> creationFunction); 
        //Here we have a helper function that we can call from the file explorer to create a file based on the extension
        static std::shared_ptr<File> CreateNewFile(const std::string &extension, std::string NewPath, std::string NewName);
        void RenderGUI(std::filesystem::directory_entry entry, ImVec2 Position, TextureAtlas& IconAtlas, const std::string& SelecteFile, bool& Renaming, bool& ShouldStopRenaming);
        virtual void OnClick(std::filesystem::directory_entry entry) {}
        virtual void OnRightClick(std::filesystem::directory_entry entry) {}
        virtual void OnDoubleClick(std::filesystem::directory_entry entry) {}
        virtual void SetIconPos() {} // Indicates the position of the file's icon on the IconAtlas.png texture
        std::string Path;
        std::string Name;
        std::string Extension;
    protected:
        glm::vec2 m_IconPos = glm::vec2(0);
        glm::vec2 m_IconSize = glm::vec2(0);
};

//For the "unsupported" files.
class Default : public File
{
    public:
        void SetIconPos() override{ 
            m_IconPos = glm::vec2(0,ICON_SIZE); 
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
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
            m_IconPos = glm::vec2(0,ICON_SIZE*2); 
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
        }
        void OnClick(std::filesystem::directory_entry entry) 
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            Editor::CurrentPath = entry.path().string();
        }
};

class LuaFile : public File
{
    public:
        void SetIconPos() override{ 
            m_IconPos = glm::vec2(ICON_SIZE,ICON_SIZE); 
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
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
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
        }
        void OnClick(std::filesystem::directory_entry entry) override
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            Engine::GetActiveScene().Load(entry.path().filename().string());
        }
};
class ImageFile : public File
{
    public:
        void SetIconPos() override{
            m_IconPos = glm::vec2(ICON_SIZE,ICON_SIZE*2);
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
        }
        void OnClick(std::filesystem::directory_entry entry) override
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            system((entry.path().string()).c_str());
        }
};
class AnimationFile : public File
{
    public:
        inline static std::filesystem::path AnimationSelectedFile = "";
        inline static SapphireRenderer::KeyFrame* SelectedKeyframe = nullptr;
        inline static std::vector<SapphireRenderer::KeyFrame*> Keyframes;
        void SetIconPos() override{
            m_IconPos = glm::vec2(ICON_SIZE,0);
            m_IconSize = glm::vec2(ICON_SIZE,ICON_SIZE); 
        }
        void OnClick(std::filesystem::directory_entry entry) override
        {

        }
        void OnDoubleClick(std::filesystem::directory_entry entry) override
        {
            AnimationSelectedFile = entry.path();
            std::ifstream fileStream(AnimationSelectedFile.string());
            for (auto &&keyframe : Keyframes)
            {
                delete keyframe;
            }
            Keyframes.clear();
            if(fileStream.is_open()){
                fileStream.seekg(0, std::ios::end);
                if(fileStream.tellg() == 0){
                    //The file is empty (so probably a new animation) so don't load anything.
                    return;
                }
            }
            //loading the new animation keyframes to be showed in the animation timeline.
            std::vector<SapphireRenderer::KeyFramePair> newKeyFrames = SapphireRenderer::Animation::readKeyFramePairsFromBinaryFile(AnimationSelectedFile.string());
            for (auto &&keyframe : newKeyFrames)
            {
                Keyframes.push_back(new SapphireRenderer::KeyFrame({keyframe.TimeStamp, *keyframe.path}));
            }
        }
};