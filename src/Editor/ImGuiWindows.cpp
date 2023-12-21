#include "UI/FileExplorer/FileExplorer.h"
#include "Editor.h"
#include "ImGuiWindows.h"
#include "Imgui/implot.h"

constexpr int MIN_ICON_SIZE = 32;

void DockSpace()
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar);
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void LogWindow()
{
    if(!(*Editor::GetWindowState("Logs"))) return;
    std::string windowName = "Logs";
    if (SapphireEngine::Logs.Size() <= 999) {
        windowName += " (" + std::to_string(SapphireEngine::Logs.Size()) + ")";
    } else {
        windowName += " (999+)";
    }
    windowName += "###LogWindow";
    ImGui::Begin(windowName.c_str(), Editor::GetWindowState("Logs"));
    ImGui::TextUnformatted(SapphireEngine::Logs.GetContent().str().c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }


    //Here I'm tryign to keep the button always visible even on scroll
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100.0f, ImGui::GetScrollY() + 20.0f));
    //I need to use a child window because otherwise ImGui will render the other child windows (the logs above) and that will cause the button to be clickable only 
    //under the logs and makes it hard to click.
    if (ImGui::Button("Clear"))
    {
        SapphireEngine::Logs.Clear();
    }

    ImGui::End();
}
std::string Name;
void Toolbar()
{
    if (ImGui::BeginMainMenuBar())
    {
        FileMenu();
        EditMenu();
        ViewMenu();
        HelpMenu();
        if(Engine::GetActiveScene().SceneFile == ""){
            ImGui::Text("No scene loaded");
        }else{
            ImGui::Text(std::string("Viewing scene: " + Engine::GetActiveScene().SceneFile).c_str());
        }
        ImGui::EndMainMenuBar();
    }

}
void FileMenu(){
    if (ImGui::BeginMenu("File"))
    {
        if(ImGui::Button("Save"))
        {
            if(Engine::GetActiveScene().SceneFile != ""){
                Engine::GetActiveScene().Save(Engine::GetActiveScene().SceneFile);
            }else{
                ImGui::OpenPopup("FileSaveMenu");
            }
        }
        if(ImGui::Button("Save As"))
        {
            ImGui::OpenPopup("FileSaveMenu");
        }
        if(ImGui::Button("Export")){
            // Engine::Get().Export();
        }
        if (ImGui::BeginPopup("FileSaveMenu"))
        {
            ImGui::InputText("Scene Name", &Name);
            if (ImGui::MenuItem("Save"))
            {
                Engine::GetActiveScene().Save(std::string(Name) + ".scene");
            }

            ImGui::EndPopup();
        }
        ImGui::EndMenu();
    }
}

void EditMenu()
{
    if (ImGui::BeginMenu("Edit"))
    {
        if(ImGui::Button("Settings"))
        {
            Editor::SetWindowState("Settings", true);
        }
        if(ImGui::Button("Preferences"))
        {
            Editor::SetWindowState("Preferences", true);
        }
        if(ImGui::Button("Project Settings"))
        {
            Editor::SetWindowState("Project Settings", true);
        }
        if(ImGui::Button("Performance"))
        {
            Editor::SetWindowState("Performance", true);
        }
        ImGui::EndMenu();
    }
    // PerformanceWindow();
    PreferencesWindow();
    ProjectSettings();
    Settings();
}
std::vector<float> FrameTimes = {0.f};
void FrameRate()
{
    if(!(*Editor::GetWindowState("FrameRate"))) return;
    if(FrameTimes.size() > 200) FrameTimes.erase(FrameTimes.begin());
    FrameTimes.push_back(1.0f / Engine::GetDeltaTime());
    ImGui::Begin("Frame rate", Editor::GetWindowState("FrameRate"));
    if(ImPlot::BeginSubplots("", 1,1, ImVec2(ImGui::GetWindowSize().x,ImGui::GetWindowSize().y)))
    {
        if(ImPlot::BeginPlot(""))
        {
            ImPlot::SetupAxis(ImAxis_X1, "Frame Number", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, "Frame Rate");
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);

            std::vector<float> LocalSamples(FrameTimes.size());
            std::generate(LocalSamples.begin(), LocalSamples.end(), [n=0]() mutable {return 1.0 * n++;});
            ImPlot::PlotLine("Frame Rate", LocalSamples.data(), FrameTimes.data(), FrameTimes.size());
            ImPlot::EndPlot();
        }
        ImPlot::EndSubplots();
    }
    ImGui::End();
}

void HelpMenu()
{
    if (ImGui::BeginMenu("Help"))
    {
        if(ImGui::Button("Documentation"))
        {

        }
        if(ImGui::Button("GitHub Repository"))
        {
            system("START https://github.com/gelis07/Sapphire-Engine");
        }
        ImGui::EndMenu();
    }
}
ImVec4 Saturate(ImVec4 StartingCol,float amount){
    return ImVec4(StartingCol.x + amount / 255.0f, StartingCol.y + amount / 255.0f, StartingCol.z + amount / 255.0f, 1);
}
void OnThemeChange()
{
    ImGuiStyle& style = ImGui::GetStyle();


    LoadedTheme[ImGuiCol_TabUnfocused] = LoadedTheme[ImGuiCol_Tab];
    LoadedTheme[ImGuiCol_TabUnfocusedActive] = LoadedTheme[ImGuiCol_Tab];
    LoadedTheme[ImGuiCol_TabActive] = Saturate(LoadedTheme[ImGuiCol_Tab], SatAmount);
    LoadedTheme[ImGuiCol_TabHovered] = Saturate(LoadedTheme[ImGuiCol_Tab], 20);

    LoadedTheme[ImGuiCol_TitleBgActive] = LoadedTheme[ImGuiCol_TitleBg], SatAmount;

    LoadedTheme[ImGuiCol_Button] = LoadedTheme[ImGuiCol_FrameBg];
    LoadedTheme[ImGuiCol_ButtonActive] = Saturate(LoadedTheme[ImGuiCol_FrameBg], SatAmount);
    LoadedTheme[ImGuiCol_ButtonHovered] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);
    LoadedTheme[ImGuiCol_FrameBgActive] = Saturate(LoadedTheme[ImGuiCol_FrameBg], SatAmount);
    LoadedTheme[ImGuiCol_FrameBgHovered] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);
    LoadedTheme[ImGuiCol_ChildBg] = Saturate(LoadedTheme[ImGuiCol_WindowBg], SatAmount);
    LoadedTheme[ImGuiCol_TextSelectedBg] = Saturate(LoadedTheme[ImGuiCol_FrameBg], 20);

    for(auto const& x : LoadedTheme)
    {
        style.Colors[x.first] = x.second;
    }

    style.FrameRounding = 4;
    style.PopupRounding = 2;
    style.GrabRounding = 1;
    style.TabRounding = 6;
    style.TabRounding = 1;
}


void ViewMenu()
{
    if(ImGui::BeginMenu("View")){
        for (auto &&Window : Editor::WindowStates)
        {
            if(ImGui::Button(Window.first.c_str()))
            {
                Window.second = !Window.second;
            }
        }
        ImGui::EndMenu();
    }
}

void PreferencesWindow()
{
    if(!(*Editor::GetWindowState("Preferences"))) return;
    ImGui::Begin("Preferences", Editor::GetWindowState("Preferences"));

    if(ImGui::BeginCombo("Theme", Editor::ThemeName.Get().c_str())){
        for(const auto &entry : std::filesystem::directory_iterator("Themes/")){
            std::string name = entry.path().filename().string();
            name = name.erase(name.size() - 5, name.size());
            if(ImGui::Selectable(name.c_str()))
            {
                Load(name);
                Editor::ThemeName.Get() = name;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Text("Options coming soon.");

    ImGui::End();
}

void ProjectSettings()
{
    if(!(*Editor::GetWindowState("Project Settings"))) return;
    ImGui::Begin("Project Settings", Editor::GetWindowState("Project Settings"));
    for (auto &&setting : Engine::SettingsVariables)
    {
        setting.second->RenderGUI(Engine::SettingsVariables);
    }
    
    ImGui::Text("More options coming soon.");

    ImGui::End();
}

void Settings()
{
    if(!(*Editor::GetWindowState("Settings"))) return;
    ImGui::Begin("Settings", Editor::GetWindowState("Settings"));

    
    ImGui::Text("Options coming soon.");


    ImGui::End();
}
static bool pressed = false;
static bool FirstTimeClicking = true; // Indicates the first time the user clicks on the SelectedObj
static glm::vec2 LastPos;
constexpr float Offset = 100.0f;
constexpr float TimeBetweenOffset = 0.1f;
float MainPoint = 0.0f;
float Pointer = 0.0f; // The currently active time stamp.
float Time = 0.0f; // the time stamp the pointer is at
bool AnimationPlaying = false;
void AnimationTimeline()
{
    bool IsShowing = ImGui::Begin("Animation Window");
    ImGui::Text(AnimationFile::AnimationSelectedFile.filename().string().c_str());
    ImGui::SameLine();
    if(ImGui::Button("Export")){
        SapphireRenderer::Animation::Export(AnimationFile::Keyframes, AnimationFile::AnimationSelectedFile.filename().string());
    }
    if(ImGui::Button("Play")){
        AnimationPlaying = !AnimationPlaying;
    }
    if(AnimationPlaying) Time += Engine::GetDeltaTime();
    float WindowPosX = ImGui::GetWindowPos().x;
    float Point = WindowPosX + MainPoint + 30;
    int i = 0;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f), IM_COL32(255, 255, 255, 255), 1.0f);
    while (Point < WindowPosX + ImGui::GetWindowSize().x)
    {
        std::stringstream ss;
        ss << i * TimeBetweenOffset;
        ImGui::GetWindowDrawList()->AddText(ImVec2(Point, ImGui::GetWindowPos().y + 20.0f), IM_COL32(255, 255, 255, 255), ss.str().c_str());
        ImGui::GetWindowDrawList()->AddLine(ImVec2(Point, ImGui::GetWindowPos().y + 50.0f), ImVec2(Point, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), IM_COL32(255, 255, 255, 255), 1.0f);
        Point += Offset;
        i++;
    }
    float TimeStamp1 = ((Time * Offset) / TimeBetweenOffset) + 30.0f;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(MainPoint + WindowPosX + TimeStamp1, ImGui::GetWindowPos().y + 50.0f), ImVec2(MainPoint + WindowPosX + TimeStamp1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), IM_COL32(0, 255, 0, 255), 1.0f);
    Pointer = (Time * Offset + 30 * TimeBetweenOffset) / TimeBetweenOffset;
    if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
    {
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        Pointer = (xpos - ImGui::GetWindowPos().x) - MainPoint;
        Time = (TimeBetweenOffset * (Pointer - 30.0f)) / Offset;
    }
    for (size_t i = 0; i < AnimationFile::Keyframes.size(); i++)
    {
        std::stringstream ss;
        float TimeStamp = ((AnimationFile::Keyframes[i]->TimeStamp * Offset) / TimeBetweenOffset) + 30.0f;
        ss << AnimationFile::Keyframes[i]->TimeStamp;
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(MainPoint + WindowPosX + TimeStamp, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f), 5.0f, IM_COL32(255,0,0,255), 12);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(MainPoint + WindowPosX + TimeStamp, ImGui::GetWindowPos().y + 50.0f), ImVec2(MainPoint + WindowPosX + TimeStamp, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), IM_COL32(200, 200, 200, 255), 1.0f);
        double CursorPosX,CursorPosY; 
        glfwGetCursorPos(glfwGetCurrentContext(), &CursorPosX,&CursorPosY);
        if(glm::length(glm::vec2(CursorPosX, CursorPosY) - glm::vec2(MainPoint + WindowPosX + TimeStamp, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2.0f)) <= 5.0f && ImGui::IsMouseClicked(0)){
            AnimationFile::SelectedKeyframe = AnimationFile::Keyframes[i];
        }
        if(glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_DELETE) == GLFW_PRESS){
            if(AnimationFile::Keyframes[i] == AnimationFile::SelectedKeyframe){
                AnimationFile::Keyframes.erase(AnimationFile::Keyframes.begin() + i);
                AnimationFile::SelectedKeyframe = nullptr;
            }
        }
    }
    
    if(ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        // {(xpos - ImGui::GetWindowPos().x) - MainPoint, ""}
        SapphireRenderer::KeyFrame* keyframe = new SapphireRenderer::KeyFrame();
        keyframe->TimeStamp = (xpos - ImGui::GetWindowPos().x) - MainPoint;
        keyframe->TimeStamp = (TimeBetweenOffset * (keyframe->TimeStamp - 30.0f)) / Offset;
        keyframe->path = "";
        AnimationFile::Keyframes.push_back(keyframe);
    }
    if(ImGui::IsWindowHovered()){
        double xpos, ypos;
        glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
        glm::vec2 CursorPos(xpos, ypos);
        glm::vec2 CursorPosToWind((CursorPos.x - ImGui::GetWindowPos().x), (ImGui::GetWindowPos().y - CursorPos.y));
        if(glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(FirstTimeClicking){
                LastPos = CursorPosToWind - glm::vec2(MainPoint, 0);
                FirstTimeClicking = false;
            }
            if(!(MainPoint > 0 && (MainPoint - (CursorPosToWind.x - LastPos.x)) < 0.0f)){
                MainPoint = CursorPosToWind.x - LastPos.x;
            }
        }
    }
    if(glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && !FirstTimeClicking) FirstTimeClicking = true;

    std::shared_ptr<File> *File = AnimationDrop.ReceiveDrop(ImGui::GetCurrentWindow());
    if (File != NULL && IsShowing)
    {
        if(File->get()->Extension == ".png"){
            double xpos, ypos;
            glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
            SapphireRenderer::KeyFrame* keyframe = new SapphireRenderer::KeyFrame();
            keyframe->TimeStamp = (xpos - ImGui::GetWindowPos().x) - MainPoint;
            keyframe->TimeStamp = (TimeBetweenOffset * (keyframe->TimeStamp - 30.0f)) / Offset;
            keyframe->path = Engine::GetMainPath() + (*File)->Path;
            AnimationFile::Keyframes.push_back(keyframe);
        }
    }
    ImGui::End();



    if(AnimationFile::SelectedKeyframe != nullptr)
    {
        ImGui::Begin("Keyframe inspector");
        ImGui::DragFloat("Timestamp", &AnimationFile::SelectedKeyframe->TimeStamp, TimeBetweenOffset, 0.0f, INFINITY, "%.2f");
        ImGui::InputText("Path",&AnimationFile::SelectedKeyframe->path);
        if(ImGui::Button("Delete")){
            AnimationFile::Keyframes.erase(AnimationFile::Keyframes.begin() + i);
            AnimationFile::SelectedKeyframe = nullptr;
        }
        ImGui::End();
    }
    AnimationPreview();
}

SapphireRenderer::Texture ActiveTexture;
SapphireRenderer::KeyFrame* ViewingKeyFrame = nullptr;

void AnimationPreview(){
    if(ViewingKeyFrame == nullptr && AnimationFile::Keyframes.size() != 0){
        ViewingKeyFrame = AnimationFile::Keyframes[0];
    }else if(ViewingKeyFrame == nullptr && AnimationFile::Keyframes.size() == 0)
        return;
    
    bool FoundImage = false;
    bool End = false;
    // for (auto &&keyframe : AnimationFile::Keyframes)
    // {
    //     if(Time > keyframe->TimeStamp){
    //         ViewingKeyFrame = keyframe;
    //         ActiveTexture.Load(ViewingKeyFrame->path, true);
    //         FoundImage = true;
    //         break;
    //     }
    // }
    for (size_t i = 0; i < AnimationFile::Keyframes.size(); i++)
    {
        SapphireRenderer::KeyFrame* keyframe = AnimationFile::Keyframes[i];
        SapphireRenderer::KeyFrame* NextKeyframe = AnimationFile::Keyframes[(i + 1) % AnimationFile::Keyframes.size()];
        if(Time >= keyframe->TimeStamp && Time < NextKeyframe->TimeStamp){
            ViewingKeyFrame = keyframe;
            ActiveTexture.Load(ViewingKeyFrame->path, true);
            FoundImage = true;
            break;
        }
        if(i == AnimationFile::Keyframes.size() - 1 && Time >= keyframe->TimeStamp){
            ViewingKeyFrame = keyframe;
            ActiveTexture.Load(ViewingKeyFrame->path, true);
            FoundImage = true;
            End = true;
            break;
        }
    }
    
    if(AnimationPlaying && End){
        Time = 0.0f;
    }else if(!AnimationPlaying && !FoundImage){
        // Image dimensions
        int width = 512;
        int height = 512;
        int channels = 3;  // Assuming RGB format

        // Create a black texture
        unsigned char* imageData = new unsigned char[width * height * channels];
        memset(imageData, 0, width * height * channels);
        ActiveTexture.SetImage(width, height, imageData);
    }
    ImGui::Begin("Animation Preview");
    float WindowWidth = ImGui::GetContentRegionAvail().x;
    float WindowHeight = ImGui::GetContentRegionAvail().y;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage(
        reinterpret_cast<ImTextureID*>(ActiveTexture.GetID()), 
        ImVec2(pos.x, pos.y), 
        ImVec2(pos.x + WindowWidth, pos.y + WindowHeight), 
        ImVec2(0, 1), 
        ImVec2(1, 0)
    );
    ImGui::End();
}

void Save(std::string name){
    std::ofstream stream("Themes/" + name +".json", std::ofstream::trunc);
    nlohmann::json Theme;
    Theme["ImGuiCol_WindowBg"] = {LoadedTheme[ImGuiCol_WindowBg].x, LoadedTheme[ImGuiCol_WindowBg].y, LoadedTheme[ImGuiCol_WindowBg].z,LoadedTheme[ImGuiCol_WindowBg].w};
    Theme["ImGuiCol_Tab"] = {LoadedTheme[ImGuiCol_Tab].x, LoadedTheme[ImGuiCol_Tab].y, LoadedTheme[ImGuiCol_Tab].z,LoadedTheme[ImGuiCol_Tab].w};
    Theme["ImGuiCol_TitleBg"] = {LoadedTheme[ImGuiCol_TitleBg].x, LoadedTheme[ImGuiCol_TitleBg].y, LoadedTheme[ImGuiCol_TitleBg].z,LoadedTheme[ImGuiCol_TitleBg].w};
    Theme["ImGuiCol_FrameBg"] = {LoadedTheme[ImGuiCol_FrameBg].x, LoadedTheme[ImGuiCol_FrameBg].y, LoadedTheme[ImGuiCol_FrameBg].z,LoadedTheme[ImGuiCol_FrameBg].w};
    Theme["SatAmount"] = SatAmount;
    stream << Theme.dump(2);
    stream.close();
}

void Load(std::string name){
    std::ifstream stream("Themes/" + name + ".json");
    nlohmann::json Theme;
    stream >> Theme;
    stream.close();
    
    LoadedTheme[ImGuiCol_WindowBg] = ImVec4(Theme["ImGuiCol_WindowBg"][0], Theme["ImGuiCol_WindowBg"][1], Theme["ImGuiCol_WindowBg"][2], Theme["ImGuiCol_WindowBg"][3]);
    LoadedTheme[ImGuiCol_Tab] = ImVec4(Theme["ImGuiCol_Tab"][0], Theme["ImGuiCol_Tab"][1], Theme["ImGuiCol_Tab"][2], Theme["ImGuiCol_Tab"][3]);
    LoadedTheme[ImGuiCol_TitleBg] = ImVec4(Theme["ImGuiCol_TitleBg"][0], Theme["ImGuiCol_TitleBg"][1], Theme["ImGuiCol_TitleBg"][2], Theme["ImGuiCol_TitleBg"][3]);
    LoadedTheme[ImGuiCol_FrameBg] = ImVec4(Theme["ImGuiCol_FrameBg"][0], Theme["ImGuiCol_FrameBg"][1], Theme["ImGuiCol_FrameBg"][2], Theme["ImGuiCol_FrameBg"][3]);
    SatAmount = Theme["SatAmount"];
    OnThemeChange();
}
std::string NewThemeName;
void ThemeMaker()
{
    if(!(*Editor::GetWindowState("ThemeMaker"))) return;
    ImGui::Begin("ThemeMaker", Editor::GetWindowState("ThemeMaker"));

    ImGui::ColorEdit4("ImGuiCol_WindowBg", (float*)&(LoadedTheme[ImGuiCol_WindowBg]));
    ImGui::ColorEdit4("ImGuiCol_Tab", (float*)&(LoadedTheme[ImGuiCol_Tab]));
    ImGui::ColorEdit4("ImGuiCol_TitleBg", (float*)&(LoadedTheme[ImGuiCol_TitleBg]));
    ImGui::ColorEdit4("ImGuiCol_FrameBg", (float*)&(LoadedTheme[ImGuiCol_FrameBg]));
    ImGui::SliderFloat("Saturate", &SatAmount, -255.0f, 255.0f);
    if(ImGui::Button("Save")){
        ImGui::OpenPopup("Save");
    }
    if(ImGui::Button("Load")){
        ImGui::OpenPopup("Load");
    }

    if(ImGui::BeginPopup("Save")){
        ImGui::InputText("Theme Name", &NewThemeName);
        if(ImGui::Button("Save")){
            Save(NewThemeName);
        }
        ImGui::EndPopup();
    }
    ImGui::End();
    OnThemeChange();
}

void TestWindow()
{

}

void PerformanceWindow()
{
    if(!(*Editor::GetWindowState("Performance"))) return;
    ImGui::Begin("Performance Profiler");
    ImGui::Text(("Delta time: " + std::to_string(Engine::GetDeltaTime()) + " (FPS: " + std::to_string(10.0f / Engine::GetDeltaTime()) + ")").c_str());
    ImGui::End();
}