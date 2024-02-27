#pragma once
#include "Objects/Objects.h"

class Scene{
    public:
        std::string SceneFile = "";
        std::vector<Object> Objects = {};
        std::vector<Object> ObjectsToAdd = {};
        std::vector<int> ObjectsToDelete = {};
        std::unordered_map<int,int> ObjectRefrences;

        ObjectRef Add(Object&& obj);
        ObjectRef Add(Object&& obj, int refID);
        void Delete(int ID);
        void Delete(Object* obj);
        void DeleteRuntime(Object* obj);

        Object LoadObj(nlohmann::ordered_json& JsonObj, int i, std::vector<ObjectRef>& o_CreatedChildren);
        // Write the json representation of the object to a stream
        void Save(const std::string FilePath);

        // Read a json representation of the object from a stream
        void Load(const std::string FilePath);
};
