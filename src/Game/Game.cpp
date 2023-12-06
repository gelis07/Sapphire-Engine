#include "Game.h"

Game::Game() : Application(glm::vec2(960,540), false, "C:/Gelis/Programs/Flappy_Bird/Assets/"), engine()
{
    engine.SetApp(this);
    std::ifstream stream(AppMainPath + "/../ProjectSettings.json");
    nlohmann::json Data;
    stream >> Data;
    for (auto &&setting : Data.items())
    {
        Engine::SettingsVariables[setting.key()]->Load(setting.value());
    }
    stream.close();
    Engine::GetActiveScene().Load("bird.scene");
}

void Game::OnUpdate(const float DeltaTime)
{
    int width,height;
    glfwGetWindowSize(window,&width, &height);
    Engine::GetCameraObject()->GetTransform()->SetSize(glm::vec3(width, height,0));
    this->DeltaTime = SapphireEngine::Clamp(0.001f, 0.5f, this->DeltaTime);
    engine.Run();
}