#define GLEW_STATIC
#include "Engine/Engine.h"
#include "Engine/StartUp.hpp"
#include "UI/ProjectManager.h"
#include "RunTime/RunTime.h"

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
        Pm.Run();
        //The engine
        // Engine::Get().Init(Pm.Run()); //Pm.Run() will return the Main Path for the Engine.
    }
    Engine::Get().Init();
    Engine::Get().Update();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}