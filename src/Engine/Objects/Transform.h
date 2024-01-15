#pragma once
#include "Utilities.hpp"
#include "Scripting/Components.h"
#define TOPIXELS * 50.0f
#define TOUNITS / 50.0f
class Object;
class Transform : public Component
{
    public:
        Transform(std::string ArgName, std::vector<glm::vec3> aPoints);
        Transform(const Transform& transform);
        void Move(const glm::vec3& translation);
        void Rotate(const float& amount);
        void UpdateModel();
        void UpdatePoints();
        //Getters and Setters.
        void SetPosition(const glm::vec3& NewPosition);
        const glm::vec3& GetPosition() const {return Position.Get();}
        glm::vec3 GetWorldPositon() const;
        void SetRotation(const float& NewRotation);
        const glm::vec3& GetRotation() const {return Rotation.Get();}

        void SetSize(const glm::vec3& NewSize);
        const glm::vec3& GetSize() const {return Size.Get();}

        const std::vector<glm::vec3>& GetPoints() const { return Points;}
        const std::vector<glm::vec3>& GetOriginalPoints() const { return OriginalPoints;}

        const glm::mat4& GetModel() const {return Model;}
        glm::mat4 Model;
        static int MoveLua(lua_State* L);
        static int LookAt(lua_State* L);
        static int SetPositionLua(lua_State* L);
        static int SetRotationLua(lua_State* L);
        static int RotateLua(lua_State* L);
        std::vector<Transform*> childrenTransforms;
        Transform* Parent = nullptr;
    private:
        std::vector<glm::vec3> OriginalPoints;
        std::vector<glm::vec3> Points;
        SapphireEngine::Vec3 Position;
        SapphireEngine::Vec3 Rotation;
        SapphireEngine::Vec3 Size;

};