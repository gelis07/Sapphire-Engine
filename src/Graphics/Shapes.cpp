#include "Shapes.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"


static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
//This is a complicated name but its just the shape constructor
Shapes::Shape::Shape(SapphireRenderer::Shader& shader) : Shader(shader), VertexArray(), VertexBuffer(), IndexBuffer()
{
    Shader = shader;
    VertexArray.Bind();
    VertexBuffer.Bind();
    IndexBuffer.Bind();
    float points[] = {
        -0.5f,-0.5f,
        0.5f,-0.5f,
        0.5f,0.5f,
        -0.5f,0.5f
    };

    VertexBuffer.AssignData(8 * sizeof(float), (GLbyte*)points, GL_STATIC_DRAW);

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    IndexBuffer.AssignData(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_STATIC_DRAW);

    VertexArray.Unbind();
    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
}

void Shapes::Shape::RenderShape(Transform& transform,const glm::vec3 &CamPos, float CameraZoom, bool OutLine ,bool WireFrame, std::function<void(SapphireRenderer::Shader& shader)> SetUpUniforms)
{
    //------------
    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->GetSize());
    m_Projection = glm::ortho( -WindowSize.x/2.0f / CameraZoom, WindowSize.x/2.0f / CameraZoom, -WindowSize.y / 2.0f / CameraZoom, WindowSize.y / 2.0f / CameraZoom, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), CamPos); 

    Shader.Bind();
    VertexArray.Bind();
    IndexBuffer.Bind();

    //Here is the standard model view projection matrix
    const glm::vec3& Pos = transform.GetPosition();
    glm::mat4 mvp = m_Projection * view * transform.GetModel();
    Shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    SetUpUniforms(Shader);

    glm::vec4 Color = glm::vec4(1);
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

    Shader.Unbind();
    VertexArray.Unbind();
    IndexBuffer.Unbind();
}

//& Should (almost a must) create a function to abstarct this repetetive code.
//? Maybe make it the default functon for the shape class?
void Shapes::Rectangle::Render(Transform& transform, const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame){
    RenderShape(transform, CamPos,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  });
            //                                       ^The rectangle doesn't have any extra uniforms
}

void Shapes::Circle::Render(Transform& transform, const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame){
    const glm::vec3& ObjectSize = transform.GetSize();
    const glm::vec3& ObjectPos = transform.GetPosition();

    glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CamPos.x, ObjectPos.y - ObjectSize.y/2 + CamPos.y);
    std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
        shader.SetUniform("RectWidth", ObjectSize.x);
        shader.SetUniform("RectHeight", ObjectSize.y);
        shader.SetUniform("StartPoint", StartPos);
        shader.SetUniform("CameraZoom", CameraZoom);
    };

    RenderShape(transform, CamPos,CameraZoom,OutLine, WireFrame, Uniforms);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}

//!Why does this exist?
void Shapes::CameraGizmo::Render(Transform& transform, const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame)
{
    RenderShape(transform, CamPos,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  });
            //                                       ^The rectangle doesn't have any extra uniforms
}
