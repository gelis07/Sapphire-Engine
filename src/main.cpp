#define GLEW_STATIC
#include "Editor/Editor.h"
#include "Editor/StartUp.hpp"
#include "UI/ProjectManager.h"
#include "Game/Game.h"
// Download geogebra at https://www.geogebra.org/download?lang=en
int main(void)
{
    if (!glfwInit())
        return -1; 
    // A little cool start up animation :)
    StartUp();
    // std::string MainPath = "";
    // {
    //     // A project manager to select the project you want to work on
    //     ProjectManager Pm;
    //     Pm.Update();
    //     MainPath = Pm.Path;
    // }
    Editor editor("C:/Gelis/Programs/Flappy_Bird/Assets/");
    editor.Update();
    // Game game;
    // game.Update();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}