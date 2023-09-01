#include "Shapes.h"
#include "Objects/Objects.h"
#include "Engine/Engine.h"


static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
//This is a complicated name but its just the shape constructor
Shapes::Shape::Shape(unsigned int sh) : m_Shader(sh)
{
    m_Shader = sh;
    GLCall(glGenVertexArrays(1, &m_VertexArray));
    GLCall(glGenBuffers(1, &m_VertexBuffer));
    GLCall(glGenBuffers(1, &m_IndexBuffer));

    GLCall(glBindVertexArray(m_VertexArray));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2,GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
    unsigned int Indices[] = {
        0,1,2,
        2,3,0
    };

    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), Indices, GL_DYNAMIC_DRAW));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));
}

void Shapes::Shape::RenderShape(std::shared_ptr<Object>& Object,std::vector<Vertex> vertices, const glm::vec3 &CamPos, float CameraZoom, bool OutLine ,bool WireFrame, std::function<void(unsigned int shader)> SetUpUniforms,bool Viewport)
{
    // int width, height;
    // glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    // const glm::vec2& WindowSize = glm::vec2(width, height);
    const glm::vec2& WindowSize = glm::vec2(Engine::Get().GetPlay().CameraObject->GetTransform()->Size.value<glm::vec3>());
    m_Projection = glm::ortho(0.0f, Viewport ? WindowSize.x / CameraZoom : WindowSize.x, 0.0f, Viewport ? WindowSize.y / CameraZoom : WindowSize.y, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Object->GetComponent<Transform>()->Position.value<glm::vec3>());
    glm::mat4 view = glm::translate(glm::mat4(1.0f), CamPos);   
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
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
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size() * 2, vertices.data()));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));


    GLCall(glUseProgram(m_Shader));
    GLCall(glBindVertexArray(m_VertexArray));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));

    //Here is the standard model view projection matrix
    glm::vec3& Pos = Object->GetComponent<Transform>()->Position.value<glm::vec3>();
    glm::mat4 mvp = m_Projection * view * model;

    GLCall(glUniformMatrix4fv(glGetUniformLocation(m_Shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
    SetUpUniforms(m_Shader);

    glm::vec4& Color = Object->GetComponent<Renderer>()->Color.value<glm::vec4>();
    if(OutLine){
        GLCall(glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), LineColor.r, LineColor.g, LineColor.b, LineColor.a));
    }else{
        GLCall(glUniform4f(glGetUniformLocation(m_Shader, "u_Color"), Color.r, Color.g, Color.b, Color.a));
    }
    if(!WireFrame){
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }else{
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    GLCall(glUseProgram(0));
    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

//& Should (almost a must) create a function to abstarct this repetetive code.
//? Maybe make it the default functon for the shape class?
void Shapes::Rectangle::Render(std::shared_ptr<Object>& Object,const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    glm::vec3& ObjectSize = Object->GetComponent<Transform>()->Size.value<glm::vec3>();
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


    RenderShape(Object,{
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, [](unsigned int shader) {  } ,Viewport);
            //                                       ^The rectangle doesn't have any extra uniforms
}

void Shapes::Circle::Render(std::shared_ptr<Object>& Object,const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
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
    std::function<void(unsigned int shader)> Uniforms = [Viewport, StartPos,CameraZoom, ObjectSize](unsigned int shader) { 
        GLCall(glUniform1f(glGetUniformLocation(shader, "RectWidth"), ObjectSize.x));
        GLCall(glUniform1f(glGetUniformLocation(shader, "RectHeight"), ObjectSize.y));
        GLCall(glUniform2f(glGetUniformLocation(shader, "StartPoint"), StartPos.x, StartPos.y));
        if(Viewport){
            GLCall(glUniform1f(glGetUniformLocation(shader, "CameraZoom"), CameraZoom));
        }else{
            GLCall(glUniform1f(glGetUniformLocation(shader, "CameraZoom"), 1));
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

void Shapes::CameraGizmo::Render(std::shared_ptr<Object>& Object,const glm::vec3 &CamPos, float CameraZoom, bool OutLine, bool WireFrame, bool Viewport)
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
            }, CamPos,CameraZoom,OutLine, WireFrame, [](unsigned int shader) {  } ,Viewport);
            //                                       ^The rectangle doesn't have any extra uniforms
}
