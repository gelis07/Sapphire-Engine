#include "FileExplorer.h"
#include "Editor.h"
#define SET_FILE(filetype) \
  static_cast<std::function<std::shared_ptr<File>()>>([]() -> std::shared_ptr<File> { return std::static_pointer_cast<File>(std::make_shared<filetype>()); })

namespace fs = std::filesystem;

const char* defautlLuaScript = R"(local SapphireEngine = require("SapphireEngine")

function OnStart()
    SapphireEngine.Log("Hello World!")
end

function OnUpdate()

end

function OnCollision()

end)";
const char* defaultScene = R"({
  "Object: 0": {
    "Name": "MainCamera",
    "Tag": "",
    "ID": 4014,
    "Children": null,
    "Components": {
      "Transform": {
        "path": "",
        "Variables": {
          "Size": [
            5690004131215169010,
            {
              "x": 31.920000076293945,
              "y": 12.640000343322754,
              "z": 0.0
            }
          ],
          "Rotation": [
            5690004131215169010,
            {
              "x": 0.0,
              "y": 0.0,
              "z": 0.0
            }
          ],
          "Position": [
            5690004131215169010,
            {
              "x": 0.0,
              "y": 0.0,
              "z": 0.0
            }
          ]
        }
      },
      "Camera": {
        "path": "",
        "Variables": {
          "Zoom": [
            17647767250183121875,
            1.0
          ],
          "BgColor": [
            16809261544032701581,
            {
              "r": 0.0,
              "g": 0.0,
              "b": 0.0,
              "a": 1.0
            }
          ]
        }
      },
      "Renderer": {
        "path": "",
        "Variables": {
          "Path": [
            1055573096244375537,
            ""
          ],
          "Color": [
            16809261544032701581,
            {
              "r": 1.0,
              "g": 1.0,
              "b": 1.0,
              "a": 1.0
            }
          ]
        }
      }
    },
    "shape": 1
  }
})";


void FileExplorer::Init()
{
  File::RegisterFile("default", SET_FILE(Default));
  File::RegisterFile("folder", SET_FILE(Folder));
  File::RegisterFile(".lua", SET_FILE(LuaFile));
  File::RegisterFile(".scene", SET_FILE(SceneFile));
  File::RegisterFile(".png", SET_FILE(ImageFile));
  File::RegisterFile(".anim", SET_FILE(AnimationFile));
  m_IconAtlas.AtlasID.Init();
  m_IconAtlas.AtlasID.Load("Assets/IconsAtlas.png");
  m_MinIconAtlas.AtlasID.Init();
  m_MinIconAtlas.AtlasID.Load("Assets/MinIconAtlas.png");
  Editor::InitWindow("FileExplorer");
}
// I need this function because std::fileysten::copy_options::overwrite_existing still causes a crash if the file exists. After some research I think its a bug from gcc.
void FileExplorer::CopyAndOverwrite(std::string &&CopyFrom, std::string &&PasteTo)
{
  if (std::filesystem::exists(PasteTo))
  {
    std::filesystem::remove(PasteTo);
  }
  std::filesystem::copy_file(CopyFrom, PasteTo, std::filesystem::copy_options::overwrite_existing);
}
std::string name;
std::optional<std::string> FileExplorer::InputFieldResult()
{
  ImGui::OpenPopup("Create File");
  if(ImGui::BeginPopup("Create File")){
    ImGui::InputText("Name", &name);
    if(ImGui::Button("Done") || Engine::app->GetInputDown(GLFW_KEY_ENTER)){
      ImGui::EndPopup();
      return name;
    }
    ImGui::EndPopup();
  }
  return std::nullopt;
}
bool Lua,Anim,Dir,Scene;

void FileExplorer::Open(std::string &path)
{
  if (*Editor::GetWindowState("FileExplorer") == false)
    return;
  ImGui::Begin("File Explorer", Editor::GetWindowState("FileExplorer"));
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 100.0f, ImGui::GetScrollY() + 20.0f));
  if (std::filesystem::path(path) != std::filesystem::path(Engine::GetMainPath()) && ImGui::Button("Back"))
  {
    path = std::filesystem::canonical(path + "/..").string() + "/";
  }
  ImVec2 Position = ImVec2(0, 30);
  int rows = 1;
  for (const auto &entry : fs::directory_iterator(path))
  {
    std::string FileName = entry.path().filename().string();
    if (m_Files.find(FileName) == m_Files.end())
    {
      m_Files[FileName] = File::CreateNewFile(entry.path().extension().string(), entry.path().string().erase(0, Engine::GetMainPath().size() - 1), FileName);
    }
    m_Files[FileName]->RenderGUI(entry, Position, m_IconAtlas, m_SelectedFile, Renaming, ShouldStopRenaming);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
      // MouseInput(entry.path());
      m_Files[FileName]->OnRightClick(entry);
      m_RightClicked = true;
      m_SelectedFile = entry.path().string(); // Getting the selected file so we can highlight it and modify it (delete it, rename, etc..)
      ImGui::OpenPopup("Context Menu");
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
      m_SelectedFile = entry.path().string(); // Getting the selected file so we can highlight it and modify it (delete it, rename, etc..)
      FileExplorerDrop.StartedDragging(m_Files[FileName]);
      AnimationDrop.StartedDragging(m_Files[FileName]);
      m_Files[FileName]->OnClick(entry);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(0))
    {
      m_Files[FileName]->OnDoubleClick(entry);
    }
    // Adjusting file's position so everything is spaced out.
    Position.x += ImGui::GetItemRectSize().x + 20.0f;
    if (ImGui::GetWindowSize().x - ImGui::GetItemRectSize().x / 1.5f < Position.x)
    {
      Position = ImVec2(0, (30 + ImGui::GetItemRectSize().y + 20.0f) * rows);
      rows++;
    }
  }
  Renaming = !ShouldStopRenaming;
  if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
  {
    ImGui::OpenPopup("Context Menu");
  }
  if(Lua || Anim || Dir || Scene){
    std::optional<std::string> FileName = InputFieldResult();
    if(FileName != std::nullopt){
      if(Anim){
        std::ofstream stream(Engine::GetMainPath() + FileName.value() +".anim", std::ofstream::trunc);
        Anim = false;
      }
      else if(Dir){
        fs::create_directory(Editor::CurrentPath + "/" + FileName.value());
        Dir = false;
      }
      else if(Scene){
        std::ofstream stream(Engine::GetMainPath() + FileName.value() + ".scene", std::ofstream::trunc);
        stream << defaultScene;
        Scene = false;
      }
      else if(Lua){
        std::ofstream stream(Engine::GetMainPath() + FileName.value() + ".lua", std::ofstream::trunc);
        stream << defautlLuaScript;
        Lua = false;
      }
    }
  }
  RightClickPopUp(m_SelectedFile);
  if (m_CopiedFilePath.string() != "")
    SapphireEngine::Log(m_CopiedFilePath.string(), SapphireEngine::Info);
  ImGui::End();
}
void FileExplorer::RightClickPopUp(const std::filesystem::path &path)
{
  if (ImGui::BeginPopup("Context Menu"))
  {
    if (ImGui::MenuItem("Create lua script"))
    {
      Lua = true;
    }
    if (ImGui::MenuItem("Create Empty Scene"))
    {
      Scene = true;
    }

    if (ImGui::MenuItem("Create Directory"))
    {
      Dir = true;
    }
    if (ImGui::MenuItem("Create Animation"))
    {
      Anim = true;
    }
    if (ImGui::MenuItem("Copy"))
    {
      m_CopiedFilePath = path;
      m_SelectedCut = false;
    }
    if (ImGui::MenuItem("Cut"))
    {
      m_CopiedFilePath = path;
      m_SelectedCut = true;
    }
    if (ImGui::MenuItem("Paste"))
    {
      fs::copy_file(m_CopiedFilePath, Editor::CurrentPath + "/" + (m_CopiedFilePath).filename().string(), fs::copy_options::overwrite_existing);
      if (m_SelectedCut)
      {
        fs::remove(m_CopiedFilePath);
      }
    }
    if (ImGui::MenuItem("Rename"))
    {
      Renaming = true;
      ShouldStopRenaming = false;
    }
    if (ImGui::MenuItem("Delete"))
    {
      fs::remove(path);
    }
    ImGui::EndPopup();
  }
}
