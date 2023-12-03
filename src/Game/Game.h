#include "Application/Application.h"
#include "Engine/Engine.h"
class Game : public Application{
    public:
        Game();
    private:
        void OnUpdate(const float DeltaTime) override;
        Engine engine;
};