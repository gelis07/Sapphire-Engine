#pragma once
#include "Objects/Objects.h"

class Scene{
    public:
        std::string SceneFile = "";
        std::vector<std::shared_ptr<Object>> Objects = {};

        // Write the json representation of the object to a stream
        void Save(const std::string FilePath);

        // Read a json representation of the object from a stream
        void Load(const std::string FilePath);

        void Hierechy(std::shared_ptr<Object> &SelectedObj);
        void CreateMenu(std::shared_ptr<Object> &SelectedObj);
};
