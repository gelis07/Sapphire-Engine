#define GLEW_STATIC
#include "Engine/Engine.h"
#include "Engine/StartUp.hpp"
#include "UI/ProjectManager.h"
// Download geogebra at https://www.geogebra.org/download?lang=en
int main(void)
{
    if (!glfwInit())
        return -1; 
    //A little cool start up animation :)
    StartUp();

    //A project manager to select the project you want to work on
    ProjectManager* Pm = new ProjectManager();

    //The engine
    Engine::Get().Init(Pm->Run()); //Pm->Run() will return the Main Path for the Engine.
    delete(Pm);
    Engine::Get().Run();
    return 0;
}