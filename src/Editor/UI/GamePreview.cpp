#include "GamePreview.h"
#include "Editor/Editor.h"
#include "Editor/DebugDraw.h"

GamePreview::GamePreview(Application* app) : WindowFBO("Game Preview"), engine()
{
    engine.SetApp(app);
    window = Engine::app->GetWindow();
}
static float TimeStep = 0;
static bool NextFrame = false;
void GamePreview::Draw()
{
    const glm::vec4& ClearColor = Engine::GetCameraObject()->GetComponent<Camera>()->BgColor.Get();
    GLCall(glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    if(!GameRunning && !Start){
        OnGameRestart();
    } 
    if(!GameRunning || (Paused && !NextFrame)){
        //If the game is not running. Just render everything.
        Renderer::Render(Engine::GetCameraObject()->GetComponent<Camera>().get(), Renderer::SceneRenderers);
    }
    if((GameRunning && !Paused) || NextFrame){
        //The game is running so the engine should start running.
        float time = glfwGetTime();
        engine.Run();
        SapphireEngine::DrawDebug(Engine::GetCameraObject()->GetComponent<Camera>()->GetView());
        TimeStep = (glfwGetTime() - time)*1000.0f;
        NextFrame = false;
    }

    //Changing the start bool to false here so all the start functions get executed
    if(GameRunning) Start = false;
}
static bool CheckForErrors()
{
    for (auto&& object : Engine::GetActiveScene().Objects)
    {
        //Should limit this only to the 
        for(auto&& component : object.GetComponents())
        {
            if(component->GetState() == nullptr) 
                continue;
            if(!ScriptingEngine::CheckLua(component->GetState(), luaL_dofile(component->GetState(), (Editor::GetMainPath() + component->GetFile()).c_str())))
            {
                return false;
            }
        }
    }
    return true;
}
void GamePreview::Extras()
{
    Editor::WindowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
    Editor::WindowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
    std::string Label;
    if (!GameRunning)
        Label = "Play";
    else
        Label = "Stop";
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 20));
    // Set the ImGui Button to play the game
    if (ImGui::Button(Label.c_str()))
    {
        if (Engine::GetActiveScene().SceneFile == "")
        {
            ImGui::OpenPopup("Save Menu");
        }
        else
        {
            if (!GameRunning)
                Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);

            if (CheckForErrors())
            {
                GameRunning = !GameRunning;
                Instrumentor::Get().BeginSession("Game");
            }
            else
                SapphireEngine::Log("Can't run program with an active lua script with an error!", SapphireEngine::Error);
        }
    }
    ImGui::SetCursorPos(ImVec2(20, 20));
    std::stringstream ss;
    ss << (TimeStep) << ", Objects: " << Engine::GetActiveScene().Objects.size();
    ImGui::Text(ss.str().c_str());
    if (GameRunning)
    {
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - 100, 20));
        // Set the ImGui Button to play the game
        if (ImGui::Button("Pause"))
        {
            Paused = !Paused;
            Instrumentor::Get().EndSession();
        }
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 - 200, 20));
        // Skip a frame while paused for debugging.
        if (ImGui::Button("Next Frame") && Paused)
        {
            NextFrame = true;
        }
    }
    if (ImGui::BeginPopup("Save Menu"))
    {
        ImGui::InputText("Scene Name", &SceneFileName, ImGuiInputTextFlags_CharsNoBlank);
        if (ImGui::MenuItem("Save") || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            if (CheckForErrors())
            {
                Engine::GetActiveScene().Save(std::string(SceneFileName) + ".scene");
                GameRunning = !GameRunning;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

}

void GamePreview::OnGameRestart()
{
    // This indicates that the game has been paused, and we should reset the start boolean so next time the user hits play
    Engine::GetActiveScene().Load(Engine::GetActiveScene().SceneFile);
    Engine::GameTime = 0.0f;
    Start = true;
    Paused = false;
}
