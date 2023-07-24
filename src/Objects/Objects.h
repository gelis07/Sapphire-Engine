#pragma once
#include <math.h>
#include "Graphics/ShaderFunc.h"
#include <GLFW/glfw3.h>
#include <memory>
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
        void LoadPrefab(std::string path,std::string FilePath, unsigned int ObjectsSize, GLFWwindow* window);

        template<typename T>
        std::shared_ptr<T> GetComponent();
        template<typename Derived>
        std::enable_if_t<std::is_base_of_v<Component, Derived>, void> AddComponent(Component* Comp);
        std::vector<std::shared_ptr<Component>> GetComponents() {return Components;}
        void SetComponents(std::vector<std::shared_ptr<Component>> Args) { Components = Args;}
        
        static std::shared_ptr<Object> CreateObject(std::vector<std::shared_ptr<Object>> &Objects, std::string &&ObjName, std::shared_ptr<Shapes::Shape> &&NewShape);

        void OnCollision(Object* other);
        void OnUpdate();
        void OnStart();

        std::string Name = "";
    private:
        std::vector<std::shared_ptr<Component>> Components;
};

template<typename Derived>
std::enable_if_t<std::is_base_of_v<Component, Derived>, void> Object::AddComponent(Component* Comp) 
{
    std::shared_ptr<Derived> PushedBackComp((Derived*)Comp);
    Components.push_back(std::shared_ptr<Derived>(PushedBackComp.get()));
    Comp = nullptr;
    delete(Comp);

    lua_State* L = PushedBackComp->GetState();
    lua_newtable(L);
    // int objectTableidx = lua_gettop(L);
    // lua_pushvalue(L, objectTableidx);

    for(auto &comp : Components){
        comp->SetLuaComponent(L);
    }
    

    lua_setglobal(L, "this");
    // luaL_newmetatable(L, "ObjectMetaTable");

    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, objectTableidx);
    // lua_settable(L, -3);

    // SetUpObject(this, L,"this");
}