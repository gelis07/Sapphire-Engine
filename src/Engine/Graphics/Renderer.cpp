#include "Renderer.h"

void Renderer::CustomRendering()
{
    if(TexturePath.Get() != ""){
        ImGui::Text(TexturePath.Get().c_str());
        std::stringstream ss;
        ss << "width: " << shape->GetTextureDimensions().x << ", height: " << shape->GetTextureDimensions().y;
        ImGui::Text(ss.str().c_str());
        if(ImGui::Button("Remove")){
            // shape->DeleteTexture();
            // TexturePath.Get() = "";
            // shape->SetShader(SapphireRenderer::BasicShader, [](SapphireRenderer::Shader& shader) {  });
        }
    }
}
int Renderer::LoadTexture(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Renderer* renderer = static_cast<Renderer*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    const char* name = luaL_checkstring(L, 2);
    renderer->shape->SelectAnimation(name);
    return 0;
}
int Renderer::SetColor(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Renderer* renderer = static_cast<Renderer*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    renderer->Color.Get() = glm::vec4(r,g,b,1);
    return 0;
}
void Renderer::Render(Camera* cam)
{
    for (auto &&rend : Shapes)
    {
        rend->shape->Render((*rend->transform), rend->Color.Get(), cam, false, [](SapphireRenderer::Shader& shader) {  });
    }
}
Renderer::Renderer(std::string File, std::string ArgName, unsigned int ArgId, bool LuaComp)
    : Component(std::move(File), std::move(ArgName), ArgId, LuaComp), Color("Color", Variables), TexturePath("Path", Variables)
{
    Color.Get() = glm::vec4(1);
    TexturePath.Get() = "";
    TexturePath.ShowOnInspector(false);
    {
        std::function<void()> OnChange = [this]() {
            shape->Load(TexturePath.Get(), true);
        };
        TexturePath.SetOnChangeFunc(OnChange);
    }
    Functions["Play"] = LoadTexture;
    Functions["SetColor"] = SetColor;
}
Renderer::Renderer(const Renderer &renderer)
: Component(std::move(""), std::move("Renderer"), 0, false), Color("Color", Variables), TexturePath("Path", Variables) 
{
    Color.Get() = renderer.Color.Get();
    TexturePath.Get() = "";
    TexturePath.ShowOnInspector(false);
    {
        std::function<void()> OnChange = [this]() {
            shape->Load(TexturePath.Get(), true);
        };
        TexturePath.SetOnChangeFunc(OnChange);
    }
    Functions["Play"] = LoadTexture;
    Functions["SetColor"] = SetColor;
}

Renderer::~Renderer()
{
    shape.reset();
}

glm::mat4 Camera::GetView()
{

    glm::mat4 view(1.0f);
    view = glm::translate(view, (Transform->GetPosition() + Transform->GetSize() / 2.0f)TOPIXELS);
    return view;
}
