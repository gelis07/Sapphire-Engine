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
    VertexBuffer.AssignData(8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    SapphireRenderer::VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 2);
    VertexArray.AddBuffer(VertexBuffer, layout);
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    IndexBuffer.AssignData(6 * sizeof(unsigned int), (GLbyte*)Indices, GL_DYNAMIC_DRAW);

    VertexArray.Unbind();
    VertexBuffer.Unbind();
    IndexBuffer.Unbind();
}

void Shapes::Shape::RenderShape(Object* Object,std::vector<Vertex> vertices, const glm::vec3 &CamPos, float CameraZoom, bool OutLine ,bool WireFrame, std::function<void(SapphireRenderer::Shader& shader)> SetUpUniforms,bool Viewport)
{
    // int width, height;
    // glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    // const glm::vec2& WindowSize = glm::vec2(width, height);
    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>());
    m_Projection = glm::ortho(0.0f, Viewport ? WindowSize.x / CameraZoom : WindowSize.x, 0.0f, Viewport ? WindowSize.y / CameraZoom : WindowSize.y, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Object->GetComponent<Transform>()->Position.value<glm::vec3>());
    glm::mat4 view = glm::translate(glm::mat4(1.0f), CamPos); 
    VertexBuffer.Bind();
    // Here im rendering an outline for the object
    if(OutLine){
        //Here everyting is happening so the outline is uniformly spaced from the object
        for (size_t i = 0; i < vertices.size(); i++)
        {
            if(vertices[i].Pos[0] < 0){
                //If the point is less than zero it should subtract so its to the left
                vertices[i].Pos[0] -= lineWidth;
            }else{
                //If the point is larger than zero it should add so its to the right
                vertices[i].Pos[0] += lineWidth;
            }
            if(vertices[i].Pos[1] < 0)
            {
                //If the point is less than zero it should subtract so its down
                vertices[i].Pos[1] -= lineWidth;
            }else{
                //If the point is larger than zero it should add so its up
                vertices[i].Pos[1] += lineWidth;
            }
        }
    }
    VertexBuffer.SubData(sizeof(float) * vertices.size() * 2, (GLbyte*)vertices.data());
    VertexBuffer.Unbind();

    Shader.Bind();
    VertexArray.Bind();
    IndexBuffer.Bind();

    //Here is the standard model view projection matrix
    glm::vec3& Pos = Object->GetComponent<Transform>()->Position.value<glm::vec3>();
    glm::mat4 mvp = m_Projection * view * model;

    Shader.SetUniform("u_MVP", 1,GL_FALSE, glm::value_ptr(mvp));
    SetUpUniforms(Shader);

    glm::vec4 Color = Object->GetComponent<Renderer>()->Color.value<glm::vec4>();
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
void Shapes::Rectangle::Render(Object* Object,const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    glm::vec3& ObjectSize = Object->GetComponent<Transform>()->Size.value<glm::vec3>();
    //Getting each point of the rectangle
    RectPoints[3] = glm::vec2(-ObjectSize.x/2, -ObjectSize.y/2);
    RectPoints[2] = glm::vec2(ObjectSize.x/2, -ObjectSize.y/2);
    RectPoints[1] = glm::vec2(ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[0] = glm::vec2(-ObjectSize.x/2, ObjectSize.y/2);

    float& ObjectRotation = Object->GetComponent<Transform>()->Rotation.value<glm::vec3>().z;
    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    Points[3] = glm::vec2((RectPoints[3].x) * cos((ObjectRotation)) + (RectPoints[3].y) * (-sin((ObjectRotation))), (RectPoints[3].x) * sin((ObjectRotation)) + (RectPoints[3].y) * cos((ObjectRotation)));
    Points[2] = glm::vec2((RectPoints[2].x) * cos((ObjectRotation)) + (RectPoints[2].y) * (-sin((ObjectRotation))), (RectPoints[2].x) * sin((ObjectRotation)) + (RectPoints[2].y) * cos((ObjectRotation)));
    Points[1] = glm::vec2((RectPoints[1].x) * cos((ObjectRotation)) + (RectPoints[1].y) * (-sin((ObjectRotation))), (RectPoints[1].x) * sin((ObjectRotation)) + (RectPoints[1].y) * cos((ObjectRotation)));
    Points[0] = glm::vec2((RectPoints[0].x) * cos((ObjectRotation)) + (RectPoints[0].y) * (-sin((ObjectRotation))), (RectPoints[0].x) * sin((ObjectRotation)) + (RectPoints[0].y) * cos((ObjectRotation)));

    RenderShape(Object,{
                {Points[0].x , Points[0].y},
                {Points[1].x, Points[1].y},
                {Points[2].x, Points[2].y},
               {Points[3].x, Points[3].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  } ,Viewport);
            //                                       ^The rectangle doesn't have any extra uniforms

            
}

void Shapes::Circle::Render(Object* Object,const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    glm::vec3& ObjectSize = Object->GetComponent<Transform>()->Size.value<glm::vec3>();
    glm::vec3& ObjectPos = Object->GetComponent<Transform>()->Position.value<glm::vec3>();
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2(ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[1] = glm::vec2(-ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[2] = glm::vec2(ObjectSize.x/2, -ObjectSize.y/2);
    RectPoints[3] = glm::vec2(-ObjectSize.x/2, -ObjectSize.y/2);

    float& ObjectRotation = Object->GetComponent<Transform>()->Rotation.value<glm::vec3>().z;
    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos((ObjectRotation)) + (RectPoints[0].y) * (-sin((ObjectRotation))), (RectPoints[0].x) * sin((ObjectRotation)) + (RectPoints[0].y) * cos((ObjectRotation)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos((ObjectRotation)) + (RectPoints[1].y) * (-sin((ObjectRotation))), (RectPoints[1].x) * sin((ObjectRotation)) + (RectPoints[1].y) * cos((ObjectRotation)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos((ObjectRotation)) + (RectPoints[2].y) * (-sin((ObjectRotation))), (RectPoints[2].x) * sin((ObjectRotation)) + (RectPoints[2].y) * cos((ObjectRotation)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos((ObjectRotation)) + (RectPoints[3].y) * (-sin((ObjectRotation))), (RectPoints[3].x) * sin((ObjectRotation)) + (RectPoints[3].y) * cos((ObjectRotation)));

    glm::vec2 StartPos(ObjectPos.x - ObjectSize.x/2 + CamPos.x, ObjectPos.y - ObjectSize.y/2 + CamPos.y);
    std::function<void(SapphireRenderer::Shader& shader)> Uniforms = [Viewport, StartPos,CameraZoom, ObjectSize](SapphireRenderer::Shader& shader) { 
        shader.SetUniform("RectWidth", ObjectSize.x);
        shader.SetUniform("RectHeight", ObjectSize.y);
        shader.SetUniform("StartPoint", StartPos);
        if(Viewport){
            shader.SetUniform("CameraZoom", CameraZoom);
        }else{
            shader.SetUniform("CameraZoom", 1);
        }
    };

    RenderShape(Object, {
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, Uniforms,Viewport);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}

void Shapes::CameraGizmo::Render(Object* Object,const glm::vec3 &CamPos, float CameraZoom, bool OutLine, bool WireFrame, bool Viewport)
{
    std::array<glm::vec2, 4> RectPoints;
    glm::vec3& ObjectSize = Object->GetComponent<Transform>()->Size.value<glm::vec3>();
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2((ObjectSize.x/2)*2, (ObjectSize.y/2)*2);
    RectPoints[1] = glm::vec2(0, (ObjectSize.y/2)*2);
    RectPoints[2] = glm::vec2((ObjectSize.x/2)*2, 0);
    RectPoints[3] = glm::vec2(0, 0);

    float& ObjectRotation = Object->GetComponent<Transform>()->Rotation.value<glm::vec3>().z;
    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos((ObjectRotation)) + (RectPoints[0].y) * (-sin((ObjectRotation))), (RectPoints[0].x) * sin((ObjectRotation)) + (RectPoints[0].y) * cos((ObjectRotation)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos((ObjectRotation)) + (RectPoints[1].y) * (-sin((ObjectRotation))), (RectPoints[1].x) * sin((ObjectRotation)) + (RectPoints[1].y) * cos((ObjectRotation)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos((ObjectRotation)) + (RectPoints[2].y) * (-sin((ObjectRotation))), (RectPoints[2].x) * sin((ObjectRotation)) + (RectPoints[2].y) * cos((ObjectRotation)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos((ObjectRotation)) + (RectPoints[3].y) * (-sin((ObjectRotation))), (RectPoints[3].x) * sin((ObjectRotation)) + (RectPoints[3].y) * cos((ObjectRotation)));


    RenderShape(Object,{
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, [](SapphireRenderer::Shader& shader) {  } ,Viewport);
            //                                       ^The rectangle doesn't have any extra uniforms
}
