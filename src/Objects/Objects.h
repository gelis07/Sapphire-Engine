#pragma once
#include "Graphics/Shapes.h"
#include "Scripting/Components.h"

class Object
{
    public:
        Object(std::string&& Name);

        void RemoveComponent(unsigned int id);
        void RenderGUI(); // render the object on the ImGUI inspector
        void Inspect();
        static void SetUpObject(Object* obj,lua_State* L,const std::string& Name);

        void SavePrefab();
        static std::shared_ptr<Object> LoadPrefab(std::string FilePath);

        template<typename T>
        std::shared_ptr<T> GetComponent();
        template<typename Derived>
        std::enable_if_t<std::is_base_of_v<Component, Derived>, void> AddComponent(Derived* Comp);
        std::vector<std::shared_ptr<Component>>& GetComponents() {return Components;}
        
        static std::shared_ptr<Object> CreateObject(std::string &&ObjName);

        void OnCollision(Object* other);
        void OnUpdate();
        void OnStart();

        unsigned int id;
        std::string Name;

        //Please use these functions because it doesn't have to search for these objects!
        std::shared_ptr<Transform>& GetTransform() {return transform;}
        std::shared_ptr<Renderer>& GetRenderer() {return renderer;} 
    private:
        std::shared_ptr<Transform> transform;
        std::shared_ptr<Renderer> renderer;
        bool CalledStart = false;
        std::vector<std::shared_ptr<Component>> Components;
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
    Object* obj = static_cast<Object*>(lua_touserdata(L, 1));

    const char* VariableName = lua_tostring(L, 2);

    for(auto &comp : obj->GetComponents()){
        if(comp->Name == std::string(VariableName))
        {
            comp->SetLuaComponent(L);
            return 1;
        }
    }

    return 0;
}
template<typename Derived>
std::enable_if_t<std::is_base_of_v<Component, Derived>, void> Object::AddComponent(Derived* Comp) 
{
    Components.push_back(std::shared_ptr<Derived>(Comp));
}