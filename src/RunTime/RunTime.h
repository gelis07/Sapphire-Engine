#pragma once
#include "Utilities.hpp"

class Object;
class Scene;
class GLFWwindow;

class RunTime
{
    public:
        static void Init(GLFWwindow* window, Scene* Scene, std::shared_ptr<Object>& CameraObject);
        static void Render(std::shared_ptr<Object> object, std::shared_ptr<Object> CameraObject);
        static void Run(Scene* Scene, std::shared_ptr<Object>& CameraObject);
        static void RunGame(GLFWwindow* window, Scene* Scene, std::shared_ptr<Object>& CameraObject);
};