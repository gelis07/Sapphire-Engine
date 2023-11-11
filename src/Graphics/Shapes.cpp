#include "Shapes.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"
#include "stb_image_write.h"

static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
//This is a complicated name but its just the shape constructor
SapphireRenderer::Shape::Shape(const SapphireRenderer::Shader& shader, std::vector<Vertex>&& Vertices, const std::string& path) : Shader(shader), VertexArray(), VertexBuffer(), IndexBuffer(), Texture(std::make_optional<SapphireRenderer::Texture>(path, true))
{
    Shader = shader;
    VertexArray.Bind();
    VertexBuffer.Bind();
    IndexBuffer.Bind();


    VertexBuffer.AssignData(16 * sizeof(float), (GLbyte*)Vertices.data(), GL_DYNAMIC_DRAW);

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    IndexBuffer.AssignData(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_STATIC_DRAW);
    Shader = SapphireRenderer::BasicShader;
    Texture = std::nullopt;

    VertexArray.Unbind();
    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
}

void SapphireRenderer::Shape::RenderShape(const Transform& transform,const glm::vec4& Color,const glm::vec3 &CamPos, const glm::mat4& view, float CameraZoom, bool OutLine ,bool WireFrame, const std::function<void(SapphireRenderer::Shader& shader)>& SetUpUniforms)
{
    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->GetSize());
    // m_Projection = glm::ortho( -WindowSize.x/2.0f / CameraZoom, WindowSize.x/2.0f / CameraZoom, -WindowSize.y / 2.0f / CameraZoom, WindowSize.y / 2.0f / CameraZoom, -1.0f, 1.0f);
    m_Projection = glm::ortho( 0.0f, WindowSize.x / CameraZoom, 0.0f, WindowSize.y / CameraZoom, -1.0f, 1.0f);
     

    Shader.Bind();
    VertexArray.Bind();
    IndexBuffer.Bind();
    //Here is the standard model view projection matrix
    const glm::vec3& Pos = transform.GetPosition();
    glm::mat4 mvp = m_Projection * view * transform.GetModel();
    Shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    if(Texture){
        Texture.value().SetAsActive();
        Texture.value().Bind();
        Shader.SetUniform("u_Texture", (int)Texture.value().GetSlot());
    }
    SetUpUniforms(Shader);

    if(OutLine){
        Shader.SetUniform("u_Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }else{
        Shader.SetUniform("u_Color", Color);
    }
    if(!WireFrame){
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }else{
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
    if(Texture) Texture.value().Unbind(); 
    Shader.Unbind();
    VertexArray.Unbind();
    IndexBuffer.Unbind();
}

void SapphireRenderer::Shape::Load(const std::string &path, bool flip)
{
    Shader = SapphireRenderer::TextureShader;
    if(Texture)
    {
        Texture.value().Load(path, true);
    }else{
        Texture = SapphireRenderer::Texture(path, true);
    }
}
const glm::vec2 SapphireRenderer::Shape::GetTextureDimensions() const
{
    if(!Texture)
        return glm::vec2(0);
    else
        return Texture.value().GetDimensions();
}


SapphireRenderer::Rectangle::Rectangle(const SapphireRenderer::Shader &shader, const std::string &path): Shape(shader,{
    {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
    {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
    {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
    {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
}, path) 
{

    ShapeType = RectangleT;
}

//? Maybe make it the default functon for the shape class?
void SapphireRenderer::Rectangle::Render(const Transform& transform,const glm::vec4& Color, const glm::vec3 &CamPos, const glm::mat4& view,float CameraZoom,bool OutLine, bool WireFrame){
    RenderShape(transform, Color,CamPos,view,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  });
            //                                       ^The rectangle doesn't have any extra uniforms
}

SapphireRenderer::Circle::Circle(const SapphireRenderer::Shader &shader): Shape(shader,{
    {glm::vec2(-0.5f, -0.5f),glm::vec2(0.0f, 0.0f)},
    {glm::vec2(0.5f, -0.5f), glm::vec2(1.0f, 0.0f)},
    {glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 1.0f)},
    {glm::vec2(-0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}
}) 
{
    ShapeType = CircleT;
}

void SapphireRenderer::Circle::Render(const Transform &transform, const glm::vec4 &Color, const glm::vec3 &CamPos, const glm::mat4 &view, float CameraZoom, bool OutLine, bool WireFrame)
{
    const glm::vec3& ObjectSize = transform.GetSize();
    const glm::vec3& ObjectPos = transform.GetPosition();

    glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CamPos.x, ObjectPos.y - ObjectSize.y/2 + CamPos.y);
    std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
        shader.SetUniform("RectWidth", ObjectSize.x);
        shader.SetUniform("RectHeight", ObjectSize.y);
        shader.SetUniform("StartPoint", StartPos);
        shader.SetUniform("CameraZoom", CameraZoom);
    };

    RenderShape(transform, Color,CamPos,view,CameraZoom,OutLine, WireFrame, Uniforms);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}