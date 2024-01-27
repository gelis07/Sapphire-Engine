#include "Renderer.h"

void Renderer::CustomRendering()
{
    if(TexturePath.Get() != ""){
        ImGui::Text(TexturePath.Get().c_str());
        std::stringstream ss;
        ss << "width: " << GetTextureDimensions().x << ", height: " << GetTextureDimensions().y;
        ImGui::Text(ss.str().c_str());
        ImGui::Text("Animations:");
        for (auto &&anim : Animations)
        {
            ImGui::Text(anim.first.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Remove")){
                delete anim.second;
                Animations.erase(anim.first);
                SelectAnimation(Animations.begin()->first);
                break;
            }
        }
        
        // if(ImGui::Button("Remove")){
        //     // shape->DeleteTexture();
        //     // TexturePath.Get() = "";
        //     // shape->SetShader(SapphireRenderer::BasicShader, [](SapphireRenderer::Shader& shader) {  });
        // }
    }
}
int Renderer::LoadTexture(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "__userdata");
    Renderer* renderer = static_cast<Renderer*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    const char* name = luaL_checkstring(L, 2);
    renderer->SelectAnimation(name);
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
void Renderer::Render(Camera* cam, const std::vector<std::shared_ptr<Renderer>>& renderers)
{
    for (auto &&rend : renderers)
    {
        glm::mat4 projection = glm::ortho( 0.0f, cam->Transform->GetSize().x TOPIXELS / cam->Zoom.Get(), 0.0f, cam->Transform->GetSize().y TOPIXELS / cam->Zoom.Get(), -1.0f, 1.0f);
    
        if(rend->CurrentAnimation) rend->CurrentAnimation.value().SelectKeyFrame(rend->VertexBuffer);

        rend->Shader.Bind();
        rend->VertexArray.Bind();
        rend->IndexBuffer.Bind();
        //Here is the standard model view projection matrix
        const glm::vec3& Pos = rend->transform->GetPosition();
        glm::mat4 mvp = projection * cam->GetView() * rend->transform->GetModel();
        rend->Shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
        if(rend->HasTexture){
            rend->Texture.SetAsActive();
            rend->Texture.Bind();
            rend->Shader.SetUniform("u_Texture", (int)rend->Texture.GetSlot());
        }
        rend->SetUpUniforms(rend->Shader);
        rend->Shader.SetUniform("u_Color", rend->Color.Get());
        if(!rend->Wireframe){
            GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }else{
            GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if(rend->HasTexture) rend->Texture.Unbind();

        rend->Shader.Unbind();
        rend->VertexArray.Unbind();
        rend->IndexBuffer.Unbind();
    }
}
void Renderer::Load(const std::string &path, bool flip)
{
    Shader = SapphireRenderer::TextureShader;
    HasTexture = true;
    Texture.Load(path, true);
}
void Renderer::SelectAnimation(const std::string &name)
{
    if(Animations.find(name) != Animations.end())
    {
        ShapeType = SapphireRenderer::RectangleT;
        // Load(Animations.at(name).GetTexturePath(), true);
        Texture = Animations.at(name)->GetTexture();
        Shader = SapphireRenderer::TextureShader;
        HasTexture = true;
        // Texture = Animations.at(name).texture;
        Animations.at(name)->SetSelectedAnimation(CurrentAnimation);
    }else
        SapphireEngine::Log("Animation with name: " + name + " was not found.", SapphireEngine::Error);
}
Renderer::Renderer(const SapphireRenderer::Shader& shader, const std::vector<Vertex>& Vertices,const std::vector<GLuint>& Indices,  const std::string& path)
    : Component("Renderer"), Color("Color", Variables), TexturePath("Path", Variables), Shader(shader), VertexArray(), VertexBuffer(), IndexBuffer()
{
    Color.Get() = glm::vec4(1);
    TexturePath.Get() = "";
    TexturePath.ShowOnInspector(false);
    {
        std::function<void()> OnChange = [this]() {
            Load(TexturePath.Get(), true);
        };
        TexturePath.SetOnChangeFunc(OnChange);
    }
    Functions["Play"] = LoadTexture;
    Functions["SetColor"] = SetColor;

    Shader = shader;
    VertexArray.Bind();
    VertexBuffer.Bind();
    IndexBuffer.Bind();

    VertexBuffer.AssignData(Vertices.size() * 4 * sizeof(float), (GLbyte*)Vertices.data(), GL_STATIC_DRAW);

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);
    IndicesAmount = Indices.size();
    IndexBuffer.AssignData(Indices.size() * sizeof(GLuint), (GLbyte*)Indices.data(), GL_STATIC_DRAW);
    // Shader = SapphireRenderer::CircleShader;

    VertexArray.Unbind();
    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
}
Renderer::Renderer(const Renderer &renderer)
: Component(std::move("Renderer")), Color("Color", Variables), TexturePath("Path", Variables) 
{
    Color.Get() = renderer.Color.Get();
    TexturePath.Get() = "";
    TexturePath.ShowOnInspector(false);
    {
        std::function<void()> OnChange = [this]() {
            Load(TexturePath.Get(), true);
        };
        TexturePath.SetOnChangeFunc(OnChange);
    }
    Functions["Play"] = LoadTexture;
    Functions["SetColor"] = SetColor;
}

Renderer::~Renderer()
{
    for (auto& animation : Animations)
    {
        delete animation.second;
    }
}

glm::mat4 Camera::GetView()
{

    glm::mat4 view(1.0f);
    view = glm::translate(view, (Transform->GetPosition() + Transform->GetSize() / 2.0f) TOPIXELS);
    return view;
}
