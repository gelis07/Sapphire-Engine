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
        inline static const std::string& GetMainPath() {return app->GetMainPath();}
        inline static void SetApp(Application* App) {app = App;}
        inline static Object* GetCameraObject() {return &m_ActiveScene.Objects[CameraObjectID];}
        inline static int CameraObjectID = -1;
    private:
        inline static Application* app;
        inline static Scene m_ActiveScene;
};
