#include "ProjectManager.h"
#include "json.hpp"
#include "Imgui/ImGuiFileDialog.h"

ProjectManager::ProjectManager()
{
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    window = glfwCreateWindow(900, 600, "Sapphire Engine Manager", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    IO = &io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(1200, 800);
    ImFontConfig config;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("Assets/font.ttf", 16.0f, &config);
    io.FontDefault = io.Fonts->Fonts.back();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
    m_IconAtlas.AtlasID = LoadTexture("Assets/IconsAtlas.png");
    m_IconAtlas.AtlasSize = glm::vec2(4096,512);
}

int ProjectsSize = 0;
bool CloseApp = true; // If the user directly closes the app from the X button GLFW should be terminated. Thats the use of this boolean
nlohmann::json Projects;

void ProjectManager::SaveJson(std::string Name){
    std::ofstream stream("Projects.sp");
    nlohmann::json Settings;
    Settings["MainPath"] = Path;
    Settings["Name"] = Name;
    ProjectsSize++;
    Projects[Name] = Settings;
    stream << Projects.dump(2);
    stream.close();
}
void replaceSubstring(std::string &original, const std::string &find, const std::string &replace) {
    size_t startPos = 0;
    while ((startPos = original.find(find, startPos)) != std::string::npos) {
        original.replace(startPos, find.length(), replace);
        startPos += replace.length();
    }
}
std::string ProjectManager::Run()
{
    std::ifstream stream("Projects.sp");
    stream >> Projects;
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        int x, y;
        glfwGetWindowSize(window, &x, &y);
        ImGui::GetIO().DisplaySize = ImVec2(x, y);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Fullscreen m_Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100, 30));
        if(ImGui::Button("New")){
            ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose Directory", nullptr, ".");
        }
        if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) 
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                bool FoundBackslash = false;
                for (size_t i = 0; i < filePath.size(); i++)
                {
                    if(filePath[i] == '\\'){
                        filePath[i] = '/';
                    }
                }
                int LastSlashIdx = 0;
                for (size_t i = filePath.size(); i >= 0; i--)
                {
                    if(filePath[i] == '/'){
                        LastSlashIdx = i;
                        break;
                    }
                }
                Path = std::string(filePath);
                filePathName = filePath.erase(0, LastSlashIdx + 1);
                std::replace(Path.begin(), Path.end(), ' ', '_');
                SaveJson(filePathName);
            }
            
            // close
            ImGuiFileDialog::Instance()->Close();
        }
        bool AreProjectsHovered = false;
        for (auto& Project : Projects.items())
        {
            std::string ProjectName = Project.value()["Name"].get<std::string>();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10);
            if(m_SelectedProject != ProjectName) ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.5f));
            else ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.2f));
            ImGui::BeginChild(ProjectName.c_str(), ImVec2(ImGui::GetWindowSize().x - 30, 50), false,ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            // Calculate the position to center the text within the child window
            ImVec2 childSize = ImGui::GetWindowSize();
            ImVec2 textSize = ImGui::CalcTextSize(ProjectName.c_str());
            float PosY = (childSize.y - textSize.y) * 0.5f;
            float IconPosY = (childSize.y - 512/14) * 0.5f;
            glm::vec4 IconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(512*7, 0), glm::vec2(512, 512), m_IconAtlas.AtlasSize); 
            ImGui::SetCursorPos(ImVec2(5, IconPosY)); // Set the cursor position to center the text
            ImGui::Image(reinterpret_cast<ImTextureID>(m_IconAtlas.AtlasID), ImVec2(512/14, 512/14), ImVec2(IconUVs.x, IconUVs.y), ImVec2(IconUVs.z, IconUVs.w));
            if(ImGui::IsWindowHovered()){
                if(ImGui::IsMouseDown(ImGuiMouseButton_Left)){
                    if(!std::filesystem::exists(Project.value()["MainPath"].get<std::string>() + "/Assets/")){
                        std::filesystem::create_directory(Project.value()["MainPath"].get<std::string>() + "/Assets/");
                    }
                    Path = Project.value()["MainPath"].get<std::string>() + "/Assets/";
                    CloseApp = false;
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
                m_SelectedProject = ProjectName;
                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
                AreProjectsHovered = true;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::SetCursorPos(ImVec2(50, PosY)); // Set the cursor position to center the text
            ImGui::TextUnformatted(ProjectName.c_str());
            ImGui::PopStyleColor();

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        if(!AreProjectsHovered)
        {
            m_SelectedProject = "";
            glfwSetCursor(window, glfwCreateStandardCursor(GLFW_ARROW_CURSOR));
        }
        
        ImGui::End();
        GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(window));
        GLCall(glfwPollEvents());
    }
    stream.close();
    glfwDestroyWindow(window);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if(CloseApp){
        glfwTerminate();
        std::exit(0);
    }

    return Path;
}
