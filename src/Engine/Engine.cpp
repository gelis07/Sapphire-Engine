#include "Engine.h"
#include <future>
#include <thread>

Engine::Engine(const std::string& mainPath)
{
    SapphireRenderer::LoadShader(const_cast<GLuint&>(SapphireRenderer::CircleShader.GetID()), "Shaders/Circle.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(SapphireRenderer::LineShader.GetID()), "Shaders/Line.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(SapphireRenderer::BasicShader.GetID()), "Shaders/Basic.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(SapphireRenderer::TextureShader.GetID()), "Shaders/Texture.glsl");
    SapphireRenderer::LoadShader(const_cast<GLuint&>(SapphireRenderer::AnimationShader.GetID()), "Shaders/Animation.glsl");

    Object Camera("MainCamera");
    std::vector<glm::vec3> points;
    points.push_back(glm::vec3(-1,-1,0));
    points.push_back(glm::vec3(1,-1,0));
    points.push_back(glm::vec3(1,1,0));
    points.push_back(glm::vec3(-1,1,0));
    Camera.AddComponent<Transform>(std::make_shared<Transform>("", "Transform", 1, std::move(points),false));
    Camera.AddComponent<LuaCamera>(std::make_shared<LuaCamera>("", "Camera", 2,  false));
    Camera.AddComponent<Renderer>(std::make_shared<Renderer>("", "Renderer", 3, false));

    Camera.GetTransform() = Camera.GetComponent<Transform>();
    Camera.GetRenderer() = Camera.GetComponent<Renderer>();
    
    Camera.GetRenderer()->shape = std::make_shared<SapphireRenderer::Shape>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices);
    Camera.GetRenderer()->shape->ShapeType = SapphireRenderer::RectangleT;

    Camera.GetRenderer()->shape->Wireframe() = true;
    m_ActiveScene.Add(std::move(Camera));
    CameraObjectID = m_ActiveScene.Objects.size()-1;
}

void Engine::Run()
{
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        m_ActiveScene.Objects[i].OnStart();
    }
    ExecuteLua();
    PhysicsSim();
    // std::future<void> physicsFuture = std::async(std::launch::async, &Engine::PhysicsSim, this);
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        Render(&m_ActiveScene.Objects[i]);
        for (size_t j = 0; j < m_ActiveScene.Objects[i].Children.size(); j++)
        {
            Render(m_ActiveScene.Objects[i].Children[j].Get());
        }
    }
    if(ShouldLoadScene != ""){
        GetActiveScene().Load(ShouldLoadScene);
        ShouldLoadScene = "";
    }
    for (size_t i = 0; i < m_ActiveScene.ObjectsToAdd.size(); i++)
    {
        int refID = m_ActiveScene.ObjectsToAdd[i].GetRefID();
        m_ActiveScene.Add(std::move(m_ActiveScene.ObjectsToAdd[i]), refID);
    }
    for (size_t i = 0; i < m_ActiveScene.ObjectsToDelete.size(); i++)
    {
        m_ActiveScene.Delete(m_ActiveScene.ObjectsToDelete[i]);
    }
    if(m_ActiveScene.ObjectsToAdd.size() != 0) m_ActiveScene.ObjectsToAdd.clear();
    if(m_ActiveScene.ObjectsToDelete.size() != 0) m_ActiveScene.ObjectsToDelete.clear();
}

void Engine::Render(Object* object)
{
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -Engine::GetCameraObject()->GetTransform()->GetPosition() + Engine::GetCameraObject()->GetTransform()->GetSize() / 2.0f);
    if(std::shared_ptr<Renderer> renderer = object->GetRenderer()) {
        if(object != Engine::GetCameraObject())
            renderer->Render(*object->GetTransform(),view, false, -Engine::GetCameraObject()->GetTransform()->GetPosition(), 1.0f);
    }
    else
        if(object->GetRenderer() = object->GetComponent<Renderer>()) 
            renderer->Render(*object->GetTransform(),view, false, -Engine::GetCameraObject()->GetTransform()->GetPosition(), 1.0f);
}

void Engine::PhysicsSim()
{
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        if(std::shared_ptr<SapphirePhysics::RigidBody> rb = m_ActiveScene.Objects[i].GetComponent<SapphirePhysics::RigidBody>()) {
            rb->Simulate(&m_ActiveScene.Objects[i], app->GetDeltaTime());
        }
    }
}

void Engine::ExecuteLua()
{
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        m_ActiveScene.Objects[i].OnUpdate();
    }
}

void Engine::Export()
{
    // if(!std::filesystem::exists(MainPath + "/../" + "Build")){
    //     std::filesystem::create_directories(MainPath + "/../" + "Build/Data");
    // }
    // //Copy pasting all the necessary dll files.
    // FileExplorer::CopyAndOverwrite("glew32.dll", MainPath + "/../" + "Build/glew32.dll");
    // FileExplorer::CopyAndOverwrite("glfw3.dll", MainPath + "/../" + "Build/glfw3.dll");
    // FileExplorer::CopyAndOverwrite("lua54.dll", MainPath + "/../" + "Build/lua54.dll");
    // if(!std::filesystem::exists(MainPath + "/../" + "Build/Shaders")){
    //     std::filesystem::copy("Shaders", MainPath + "/../" + "Build/Shaders");
    // }
    // /*
    // I know that this solution feels kinda cheap but I'm not really interested in making the most optimized builds
    // And all of that stuff, so I found a quick solution that works for the current state of the engine and allows me to
    // focus on the other stuff of the engine.
    // */
    // FileExplorer::CopyAndOverwrite("Sapphire-Engine-Runtime.exe", MainPath + "/../" + "Build/Game.exe");
    // for (auto &&object : m_ActiveScene.Objects)
    // {
    //     for (auto &&component : object->GetComponents())
    //     {
    //         if(component->GetState() != nullptr){
    //             system(("C:/Users/bagge/Downloads/lua-5.4.2_Win64_bin/luac54.exe -o " + MainPath + "/../" + "/Build/Data/"+ component->GetFile() + " " + MainPath + component->GetFile()).c_str());
    //         }
    //     }
    // }
    // for(const auto &file : FileExplorer::GetFiles()){
    //     if(file.second->Name.erase(0, file.second->Name.size() - 5) != "scene") continue;
    //     FileExplorer::CopyAndOverwrite(MainPath + file.second->Path,MainPath + "/../" + "Build/Data/" + file.second->Path);
    // }
    
}

Scene& Engine::GetActiveScene()
{
    return m_ActiveScene;
}

