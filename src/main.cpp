#define GLEW_STATIC
#include "Engine/Engine.h"
#include "Engine/StartUp.hpp"
#include "UI/ProjectManager.h"
#include "RunTime/RunTime.h"
#define EXPORT 0

// Download geogebra at https://www.geogebra.org/download?lang=en
int main(void)
{
    if (!glfwInit())
        return -1; 

    // A little cool start up animation :)
    StartUp();

    {
    // A project manager to select the project you want to work on
    ProjectManager Pm;

    //The engine
    Engine::Get().Init(Pm.Run()); //Pm->Run() will return the Main Path for the Engine.
    }
    Engine::Get().Run();


    // RunTime::RunGame(glfwGetCurrentContext(), Engine::Get().GetActiveScene(), Engine::Get().GetPlay().CameraObject);

    return 0;
}