#include "ProjectManager.h"
#include "json.hpp"
#include "Imgui/ImGuiFileDialog.h"


constexpr int ICON_SIZE = 32;
ProjectManager::ProjectManager() : Application(glm::vec2(900,600), false)
{
    stbi_set_flip_vertically_on_load(false);
    m_IconAtlas.AtlasID.Init();
    m_IconAtlas.AtlasID.Load("Assets/MinIconAtlas.png");
    std::ifstream stream("Projects.sp");
    stream >> Projects;
    stream.close();
}

void ProjectManager::OnUpdate(const float DeltaTime)
{
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
        float IconPosY = (childSize.y - ICON_SIZE) * 0.5f;
        glm::vec4 IconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(ICON_SIZE*3, 0), glm::vec2(ICON_SIZE, ICON_SIZE), m_IconAtlas.AtlasID.GetDimensions()); 
        ImGui::SetCursorPos(ImVec2(5, IconPosY)); // Set the cursor position to center the text
        ImGui::Image(reinterpret_cast<ImTextureID>(m_IconAtlas.AtlasID.GetID()), ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(IconUVs.x, IconUVs.y), ImVec2(IconUVs.z, IconUVs.w));
        
        if(ImGui::IsWindowHovered()){
            OnHover(IconPosY, ShouldDeleteProject, Project, ProjectExists, ProjectName, AreProjectsHovered);
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
}

void ProjectManager::OnHover(float IconPosY, std::string &ShouldDeleteProject, const nlohmann::json_abi_v3_11_2::detail::iteration_proxy_value<nlohmann::json_abi_v3_11_2::detail::iter_impl<nlohmann::json_abi_v3_11_2::ordered_json>> &Project, bool ProjectExists, std::string &ProjectName, bool &AreProjectsHovered)
{
    glm::vec4 BinIconUVs = SapphireEngine::LoadIconFromAtlas(glm::vec2(ICON_SIZE * 4, 0), glm::vec2(ICON_SIZE, ICON_SIZE), m_IconAtlas.AtlasID.GetDimensions());
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - ICON_SIZE - 40, IconPosY)); // Set the cursor position to center the text
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.3));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.6));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(m_IconAtlas.AtlasID.GetID()), ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(BinIconUVs.x, BinIconUVs.y), ImVec2(BinIconUVs.z, BinIconUVs.w)))
        ;
    if (ImGui::IsItemClicked())
    {
        ShouldDeleteProject = Project.key();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    if (!ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (!ProjectExists)
            {
                ImGui::OpenPopup("Warning");
            }
            else
            {
                // Project exists!
                if (!std::filesystem::exists(Project.value()["MainPath"].get<std::string>() + "/Assets/"))
                {
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

void ProjectManager::OnExit()
{
    m_IconAtlas.AtlasID.Unbind();
    if(CloseApp){
        glfwTerminate();
        std::exit(0);
    }

}

int ProjectsSize = 0;
bool CloseApp = true; // If the user directly closes the app from the X button GLFW should be terminated. Thats the use of this boolean
nlohmann::ordered_json Projects;

void ProjectManager::SaveJson(std::string Name){
    std::ofstream stream("Projects.sp");
    nlohmann::ordered_json Settings;
    Settings["MainPath"] = Path;
    Settings["Name"] = Name;
    ProjectsSize++;
    Projects[Name] = Settings;
    stream << Projects.dump(2);
    stream.close();
}
