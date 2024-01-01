#pragma once
#include "Graphics/Shapes.h"
#include "Scripting/Components.h"
#include "Transform.h"
#include "PhysicsEngine/RigidBody.h"
#include "Graphics/Renderer.h"
#define null_ref -1

class Object
{
    public:
        Object(std::string&& Name);
        ~Object();
        void RemoveComponent(unsigned int id);
        void RenderGUI(); // render the object on the ImGUI inspector
        void Inspect();
        static int RemoveComponent(lua_State* L);
        static void SetUpObject(Object* obj,lua_State* L,const std::string& Name);

        void SavePrefab();
        static Object* LoadPrefab(std::string FilePath);

        template<typename T>
        std::shared_ptr<T> GetComponent();
        template<typename Derived>
        std::enable_if_t<std::is_base_of_v<Component, Derived>, void> AddComponent(const std::shared_ptr<Derived>& Comp);
        std::vector<std::shared_ptr<Component>>& GetComponents() {return Components;}
        
        static Object* CreateObject(std::string &&ObjName);
        static ObjectRef CreateObjectRuntime(std::string &&ObjName);
        static void Delete(int id);

        void OnCollision(Object* other);
        void OnUpdate();
        void OnStart();
        static int SetActive(lua_State* L);
        unsigned int id;
        std::string Name;
        std::string Tag;
        std::vector<ObjectRef> Children = {};
        ObjectRef Parent = ObjectRef(null_ref);
        bool Active = true;
        //Please use these functions because it doesn't have to search for these objects!
        std::shared_ptr<Transform>& GetTransform() {return transform;}
        std::shared_ptr<Renderer>& GetRenderer() {return renderer;} 
        std::shared_ptr<SapphirePhysics::RigidBody>& GetRb() {return rb;} 
        const std::shared_ptr<Transform>& GetTransform() const {return transform;}
        const std::shared_ptr<Renderer>& GetRenderer() const {return renderer;} 
        const std::shared_ptr<SapphirePhysics::RigidBody>& GetRb() const {return rb;} 
        std::vector<std::shared_ptr<Component>> Components;
        void SetRefID(int refID) {RefID = refID;}
        const int& GetRefID() {return RefID;}
        ObjectRef GetRef() {return ObjectRef(RefID);}
    private:
        std::shared_ptr<Transform> transform;
        std::shared_ptr<Renderer> renderer;
        std::shared_ptr<SapphirePhysics::RigidBody> rb;
        bool m_CalledStart = false;
        int RefID;
};

template <typename T>
std::shared_ptr<T> Object::GetComponent()
{
    for (std::shared_ptr<Component> component : Components)
    {
        if (std::shared_ptr<T> SpecificComponent = std::dynamic_pointer_cast<T>(component))
        {
            return SpecificComponent;
        }
    }
    return nullptr;
}
static int GetComponentFromObject(lua_State* L) {
    ObjectRef* obj = static_cast<ObjectRef*>(lua_touserdata(L, 1));
    if((*obj).Get() == nullptr){
        SapphireEngine::Log("The object is null!", SapphireEngine::Info);
        return 0;
    }
    const char* VariableNameC = lua_tostring(L, 2);
    std::string VariableName = std::string(VariableNameC);

    if(VariableName == "Name"){
        lua_pushstring(L, (*obj)->Name.c_str());
        return 1;
    }
    if(VariableName == "SetActive"){
        lua_pushcfunction(L, Object::SetActive);
        return 1;
    }
    if(VariableName == "RemoveComponent"){
        lua_pushcfunction(L, Object::RemoveComponent);
        return 1;
    }
    for(auto &comp : (*obj)->GetComponents()){
        if(comp->Name == VariableName)
        {
            comp->SetLuaComponent(L);
            return 1;
        }
    }

    return 0;
}
template<typename Derived>
std::enable_if_t<std::is_base_of_v<Component, Derived>, void> Object::AddComponent(const std::shared_ptr<Derived>& Comp) 
{
    Components.push_back(std::move(Comp));
}