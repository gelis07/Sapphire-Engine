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
    stbi_set_flip_vertically_on_load(false);
    m_IconAtlas.AtlasID.Init();
    m_IconAtlas.AtlasID.Load("Assets/IconsAtlas.png");
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
    stream.close();
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
        std::string ShouldDeleteProject = "";
        for (auto& Project : Projects.items())
        {
            ImVec4 TextColor = ImVec4(1,1,1,1);
            bool ProjectExists;
            if(!(ProjectExists = std::filesystem::exists(Project.value()["MainPath"]))){
                TextColor = ImVec4(0.6f, 0.6f, 0.6f, 1);
            }
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
            glm::vec4 IconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(512*7, 0), glm::vec2(512, 512), m_IconAtlas.AtlasID.GetDimensions()); 
            ImGui::SetCursorPos(ImVec2(5, IconPosY)); // Set the cursor position to center the text
            ImGui::Image(reinterpret_cast<ImTextureID>(m_IconAtlas.AtlasID.GetID()), ImVec2(512/14, 512/14), ImVec2(IconUVs.x, IconUVs.y), ImVec2(IconUVs.z, IconUVs.w));
            
            if(ImGui::IsWindowHovered()){
                glm::vec4 BinIconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(512*8, 0), glm::vec2(512, 512), m_IconAtlas.AtlasID.GetDimensions()); 
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 512/14 - 40, IconPosY)); // Set the cursor position to center the text
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,1,1,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.3));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1,1,1,0.6));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
                if(ImGui::ImageButton(reinterpret_cast<ImTextureID>(m_IconAtlas.AtlasID.GetID()), ImVec2(512/14, 512/14), ImVec2(BinIconUVs.x, BinIconUVs.y), ImVec2(BinIconUVs.z, BinIconUVs.w)));
                if(ImGui::IsItemClicked()){
                    ShouldDeleteProject = Project.key();
                }
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                if(!ImGui::IsItemHovered()){
                    if(ImGui::IsMouseDown(ImGuiMouseButton_Left)){
                        if(!ProjectExists){
                            ImGui::OpenPopup("Warning");
                        }else{
                            //Project exists!
                            if(!std::filesystem::exists(Project.value()["MainPath"].get<std::string>() + "/Assets/")){
                                std::filesystem::create_directory(Project.value()["MainPath"].get<std::string>() + "/Assets/");
                            }
                            Path = Project.value()["MainPath"].get<std::string>() + "/Assets/";
                            CloseApp = false;
                            glfwSetWindowShouldClose(window, GLFW_TRUE);
                        }
                    }
                    m_SelectedProject = ProjectName;
                    glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
                    AreProjectsHovered = true;
                }
            }



            if(ImGui::BeginPopupModal("Warning", nullptr, ImGuiWindowFlags_NoResize))
            {
                ImGui::Text(("Couldn't find path: " + Project.value()["MainPath"].get<std::string>()).c_str());
                ImGui::SetCursorPosX(ImGui::GetWindowSize().x - ImGui::CalcTextSize("Got it!").x - 30);
                if(ImGui::Button("Got it!")){
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }


            ImGui::SetCursorPos(ImVec2(65, PosY - ImGui::CalcTextSize(ProjectName.c_str()).y + 5)); // Set the cursor position to center the text
            ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
            ImGui::TextUnformatted(ProjectName.c_str());
            ImGui::PopStyleColor();

            ImGui::SetCursorPos(ImVec2(65, PosY + ImGui::CalcTextSize(ProjectName.c_str()).y - 5)); // Set the cursor position to center the text
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(TextColor.x - 0.3f,TextColor.y - 0.3f,TextColor.z - 0.3f,1));
            ImGui::TextUnformatted((Project.value()["MainPath"].get<std::string>()).c_str());
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
        if(ShouldDeleteProject != "")
        {
            Projects.erase(ShouldDeleteProject);
            {
                std::ofstream stream("Projects.sp");
                stream << Projects.dump(2);
                stream.close();
            }
            std::ifstream stream("Projects.sp");
            stream >> Projects;
            stream.close();
            ShouldDeleteProject = "";
        }
        
        ImGui::End();
        GLCall(glClearColor(0.3f, 0.5f, 0.4f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GLCall(glfwSwapBuffers(window));
        GLCall(glfwPollEvents());
    }
    m_IconAtlas.AtlasID.Unbind();
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
