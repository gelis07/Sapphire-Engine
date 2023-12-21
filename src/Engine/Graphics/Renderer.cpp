#include "Renderer.h"
void Renderer::Render(const Transform& transform, const glm::mat4& view, bool&& IsSelected ,glm::vec3 CameraPos,float CameraZoom)
{
    //That means that the object is an empty
    if(shape == nullptr) return;
    // Here it renders the object's outline to indicate that the current object is selected
    if(IsSelected){
        if(shape->ShapeType == SapphireRenderer::RectangleT){
            shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,false, [](SapphireRenderer::Shader& shader) {  });
        }else if(shape->ShapeType == SapphireRenderer::CircleT){
            const glm::vec3& ObjectSize = transform.GetSize();
            const glm::vec3& ObjectPos = transform.GetPosition();

            glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CameraPos.x, ObjectPos.y - ObjectSize.y/2 + CameraPos.y);
            std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
                shader.SetUniform("RectWidth", ObjectSize.x);
                shader.SetUniform("RectHeight", ObjectSize.y);
                shader.SetUniform("StartPoint", StartPos);
                shader.SetUniform("CameraZoom", CameraZoom);
            };
            shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,true, Uniforms);
        }
    }

    if(shape->ShapeType == SapphireRenderer::RectangleT){
        shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,false, [](SapphireRenderer::Shader& shader) {  });
    }else{
        const glm::vec3& ObjectSize = transform.GetSize();
        const glm::vec3& ObjectPos = transform.GetPosition();

        glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CameraPos.x, ObjectPos.y - ObjectSize.y/2 + CameraPos.y);
        std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
            shader.SetUniform("RectWidth", ObjectSize.x);
            shader.SetUniform("RectHeight", ObjectSize.y);
            shader.SetUniform("StartPoint", StartPos);
            shader.SetUniform("CameraZoom", CameraZoom);
        };
        shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,false, Uniforms);
    }
}

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