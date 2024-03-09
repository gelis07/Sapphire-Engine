#include "Game.h"

Game::Game() : Application(glm::vec2(1100,640), false, "GameAssets/"), engine()
{
    engine.SetApp(this);
    std::ifstream stream(AppMainPath + "/../ProjectSettings.json");
    nlohmann::ordered_json Data;
    stream >> Data;
    for (auto &&setting : Data.items())
    {
        Engine::SettingsVariables[setting.key()]->Load(setting.value());
    }
    stream.close();
    Engine::GetActiveScene().Load("Test.scene");
}

void Game::OnUpdate(const float DeltaTime)
{
    int width,height;
    glfwGetWindowSize(window,&width, &height);
    Engine::GetCameraObject()->GetComponent<Transform>()->SetSize(glm::vec3(width, height,0) TOUNITS);
    GLCall(glViewport(0, 0, width, height));
    this->DeltaTime = SapphireEngine::Clamp(0.001f, 0.5f, this->DeltaTime);
    engine.Run();
}