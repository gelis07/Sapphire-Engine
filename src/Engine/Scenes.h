#pragma once
#include "Objects/Objects.h"

class Scene{
    public:
        std::string SceneFile = "";
        std::vector<std::shared_ptr<Object>> Objects = {};

        // Write the json representation of the object to a stream
        void Save(const std::string FilePath,const std::string& MainPath);

        // Read a json representation of the object from a stream
        void Load(const std::string FilePath,const std::string& MainPath, GLFWwindow* window);

        void Hierechy(std::shared_ptr<Object> &SelectedObj);
        void CreateMenu(std::shared_ptr<Object> &SelectedObj);
};

Scene* GetActiveScene();