#pragma once
#include "Application/Application.h"
#include "Engine/Engine.h"


class Editor : public Application{
    public:

        const std::filesystem::path& GetMainPath();
        void OnStart() override;
        void OnUpdate(float DeltaTime) override;
        void OnExit() override;
    private:
        std::filesystem::path MainPath;
        Scene ActiceScene;
};