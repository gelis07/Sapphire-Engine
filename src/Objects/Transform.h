#pragma once
#include "Utilities.hpp"
#include "Scripting/Components.h"

class Object;
class Transform : public Component
{
    public:
        Transform(std::string File, std::string ArgName, unsigned int ArgId, Object* obj,std::vector<glm::vec3> aPoints, bool LuaComp = false);
        void Move(const glm::vec3& translation);
        void Rotate(const float& amount);
        void UpdateModel();
        void UpdatePoints();
        //Getters and Setters.
        void SetPosition(const glm::vec3& NewPosition);
        const glm::vec3& GetPosition() const {return Position.Get();}

        void SetRotation(const float& NewRotation);
        const glm::vec3& GetRotation() const {return Rotation.Get();}

        void SetSize(const glm::vec3& NewSize);
        const glm::vec3& GetSize() const {return Size.Get();}

        const std::vector<glm::vec3>& GetPoints() const { return Points;}
        const std::vector<glm::vec3>& GetOriginalPoints() const { return OriginalPoints;}

        const glm::mat4& GetModel() const {return Model;}
        glm::mat4 Model;
    private:
        std::vector<glm::vec3> OriginalPoints;
        std::vector<glm::vec3> Points;
        SapphireEngine::Vec3 Position;
        SapphireEngine::Vec3 Rotation;
        SapphireEngine::Vec3 Size;
};