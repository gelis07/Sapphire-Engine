#pragma once
#include "Graphics/Shapes.h"
#include "Scripting/Components.h"

class Object
{
    public:
        unsigned int id;
        bool CalledStart = false;
        Object(std::string&& Name);

        void RemoveComponent(unsigned int id);
        // void Run();
        void RenderGUI(); // render the object on the ImGUI inspector
        void Inspect();
        static void SetUpObject(Object* obj,lua_State* L,std::string Name);

        void SavePrefab(std::string path);
        void LoadPrefab(std::string path,std::string FilePath, unsigned int ObjectsSize);

        template<typename T>
        std::shared_ptr<T> GetComponent();
        template<typename Derived>
        std::enable_if_t<std::is_base_of_v<Component, Derived>, void> AddComponent(Derived* Comp);
        std::vector<std::shared_ptr<Component>>& GetComponents() {return Components;}
        
        static std::shared_ptr<Object> CreateObject(std::vector<std::shared_ptr<Object>> &Objects, std::string &&ObjName);

        void OnCollision(Object* other);
        void OnUpdate();
        void OnStart();

        std::string Name = "";
    private:
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

    if(Components.back()->GetState() == nullptr) return;
    lua_State* L = Components.back()->GetState();
    lua_pushlightuserdata(L, this);
    luaL_newmetatable(L, "ObjectMetaTable");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, GetComponentFromObject);
    lua_settable(L, -3);

    lua_setmetatable(L, -2);
    lua_setglobal(L, Name.c_str());
}
