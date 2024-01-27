#pragma once
#include <iostream>
#include "Engine/Scenes.h"
#include <array>
#include "Application/Application.h"


class Engine{
    public:
        Engine(const Engine&) = delete;
        Engine(const std::string& mainPath = "");
        void Export();
        static Scene& GetActiveScene();
        inline static const float& GetDeltaTime() {return app->GetDeltaTime();}
        void Run();
        void Render(Object* object);
        void PhysicsSim();
        void ExecuteLua();
        inline static std::unordered_map<std::string, SapphireEngine::Variable*> SettingsVariables;
        inline static const std::string& GetMainPath() {return app->GetMainPath();}
        inline static void SetApp(Application* App) {app = App;}
        inline static Object* GetCameraObject() {return &m_ActiveScene.Objects[CameraObjectID];}
        inline static int CameraObjectID = -1;
        inline static Application* app;
        inline static bool SkipFrame = false;
        inline static std::string ShouldLoadScene = "";
        inline static std::vector<std::string> Tags = {"Player", "Enemy", "Ground"};
        inline static float GameTime;
    private:
        inline static Scene m_ActiveScene;
};
