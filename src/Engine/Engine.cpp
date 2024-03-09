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
    SapphireRenderer::shaders.push_back(&SapphireRenderer::CircleShader);
    SapphireRenderer::shaders.push_back(&SapphireRenderer::LineShader);
    SapphireRenderer::shaders.push_back(&SapphireRenderer::BasicShader);
    SapphireRenderer::shaders.push_back(&SapphireRenderer::TextureShader);
    SapphireRenderer::shaders.push_back(&SapphireRenderer::AnimationShader);
    Object CameraObj("MainCamera");
    CameraObj.AddComponent<Camera>(std::make_shared<Camera>("Camera", CameraObj.GetRef()));
    CameraObj.GetComponent<Camera>()->Transform = CameraObj.GetComponent<Transform>();
    m_ActiveScene.Add(std::move(CameraObj));

    CameraObjectID = m_ActiveScene.Objects.size()-1;
    InitCustomComponents();
}
void Engine::InitCustomComponents(){
    Component::RegisterComponentType<Renderer>([](Object* obj) { 
        std::shared_ptr<Renderer> rend = std::make_shared<Renderer>(SapphireRenderer::BasicShader, SapphireRenderer::RectangleVertices, SapphireRenderer::RectangleIndices, SapphireRenderer::RectangleT, obj->GetRef());
        rend->transform = obj->GetComponent<Transform>();
        obj->AddComponent(rend);
        Renderer::SceneRenderers.push_back(rend);
        return rend; 
    }, "Renderer");
    Component::RegisterComponentType<SapphirePhysics::RigidBody>([](Object* obj) { 
        std::shared_ptr<SapphirePhysics::RigidBody> rb = std::make_shared<SapphirePhysics::RigidBody>(SapphireRenderer::RectangleT, obj->GetRef());
        rb->transform = obj->GetComponent<Transform>().get();
        obj->AddComponent(rb);
        rb->Init();
        SapphirePhysics::RigidBody::Rigibodies.push_back(rb.get());
        return rb; 
    }, "Rigidbody");
    Component::RegisterComponentType<Camera>([](Object* obj) { 
        std::shared_ptr<Camera> camera = std::make_shared<Camera>("Camera", obj->GetRef());
        obj->AddComponent(camera);
        return camera; 
    }, "Camera");
    Component::RegisterComponentType<Transform>([](Object* obj) { 
        std::vector<glm::vec3> points;
        points.push_back(glm::vec3(-0.5f,-0.5f,0));
        points.push_back(glm::vec3(0.5f,-0.5f,0));
        points.push_back(glm::vec3(0.5f,0.5f,0));
        points.push_back(glm::vec3(-0.5f,0.5f,0));
        std::shared_ptr<Transform> transform = std::make_shared<Transform>("Transform",std::move(points), obj->GetRef());
        obj->AddComponent(transform);
        return transform; 
    }, "Transform");
}
float TimeAccumulator = 0.0f;
void Engine::Run()
{
    PROFILE_FUNC();
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        m_ActiveScene.Objects[i].OnStart();
    }
    TimeAccumulator += GetDeltaTime();
    GameTime += GetDeltaTime();
    ExecuteLua();
    {
        PROFILE_SCOPE("Physics Simulation");
        SapphirePhysics::RigidBody::Run();
    }
    // std::future<void> physicsFuture = std::async(std::launch::async, &Engine::PhysicsSim, this);
    Renderer::Render(Engine::GetCameraObject()->GetComponent<Camera>().get(), Renderer::SceneRenderers);
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
    for (auto &&compPair : Object::ComponentsToAdd)
    {
        if (compPair.second->GetLuaVariables())
            compPair.first->AddComponent<Component>(compPair.second);
    }
    
    if(m_ActiveScene.ObjectsToAdd.size() != 0) m_ActiveScene.ObjectsToAdd.clear();
    if(m_ActiveScene.ObjectsToDelete.size() != 0) m_ActiveScene.ObjectsToDelete.clear();
    if(Object::ComponentsToAdd.size() != 0) Object::ComponentsToAdd.clear();
}
void Engine::ExecuteLua()
{
    PROFILE_FUNC();
    for (size_t i = 0; i < m_ActiveScene.Objects.size(); i++)
    {
        if(!m_ActiveScene.Objects[i].Active) continue;
        m_ActiveScene.Objects[i].OnUpdate();
    }
}

Scene& Engine::GetActiveScene()
{
    return m_ActiveScene;
}

