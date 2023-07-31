#include "Shapes.h"
#include "Objects/Objects.h"

static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
//This is a complicated name but its just the shape constructor
Shapes::Shape::Shape(unsigned int sh, std::shared_ptr<Object>& NewObj) : shader(sh), Obj(NewObj) 
{
    shader = sh;
    GLCall(glGenVertexArrays(1, &VertexArray));
    GLCall(glGenBuffers(1, &VertexBuffer));
    GLCall(glGenBuffers(1, &IndexBuffer));

    GLCall(glBindVertexArray(VertexArray));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer));
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

void Shapes::Shape::RenderShape(std::vector<Vertex> vertices, const glm::vec3 &CamPos, float CameraZoom, bool OutLine ,bool WireFrame, std::function<void(unsigned int shader)> SetUpUniforms,bool Viewport)
{
    proj = glm::ortho(0.0f, Viewport ? SCREEN_WIDTH / CameraZoom : SCREEN_WIDTH, 0.0f, Viewport ? SCREEN_HEIGHT / CameraZoom : SCREEN_HEIGHT, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, GetVariable(Obj->GetComponent<Transform>(), "Position", glm::vec3));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), CamPos);
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer));
    
    // Here im rendering an outline for the object
    if(WireFrame){
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


    GLCall(glUseProgram(shader));
    GLCall(glBindVertexArray(VertexArray));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer));

    //Here is the standard model view projection matrix
    glm::vec3& Pos = GetVariable(Obj->GetComponent<Transform>(), "Position", glm::vec3);
    glm::mat4 mvp = proj * view * model;

    GLCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
    SetUpUniforms(shader);

    glm::vec4& Color = GetVariable(Obj->GetComponent<Renderer>(), "Color", glm::vec4);
    if(WireFrame){
        GLCall(glUniform4f(glGetUniformLocation(shader, "u_Color"), LineColor.r, LineColor.g, LineColor.b, LineColor.a));
    }else{
        GLCall(glUniform4f(glGetUniformLocation(shader, "u_Color"), Color.r, Color.g, Color.b, Color.a));
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

void Shapes::Rectangle::Render(const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    glm::vec2& ObjectSize = GetVariable(Obj->GetComponent<Transform>(), "Size", glm::vec2);
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2(ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[1] = glm::vec2(-ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[2] = glm::vec2(ObjectSize.x/2, -ObjectSize.y/2);
    RectPoints[3] = glm::vec2(-ObjectSize.x/2, -ObjectSize.y/2);

    float& ObjectRotation = GetVariable(Obj->GetComponent<Transform>(), "Rotation", glm::vec3).z;
    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[0].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[0].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[0].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[1].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[1].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[1].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[2].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[2].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[2].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[3].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[3].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[3].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));


    RenderShape({
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, [](unsigned int shader) {  } ,Viewport);
            //                                       ^The rectangle doesn't have any extra uniforms
}

void Shapes::Circle::Render(const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    glm::vec2 ObjectSize = GetVariable(Obj->GetComponent<Transform>(), "Size", glm::vec2);
    glm::vec3 ObjectPos = GetVariable(Obj->GetComponent<Transform>(), "Position", glm::vec3);
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2(ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[1] = glm::vec2(-ObjectSize.x/2, ObjectSize.y/2);
    RectPoints[2] = glm::vec2(ObjectSize.x/2, -ObjectSize.y/2);
    RectPoints[3] = glm::vec2(-ObjectSize.x/2, -ObjectSize.y/2);

    float& ObjectRotation = GetVariable(Obj->GetComponent<Transform>(), "Rotation", glm::vec3).z;
    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[0].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[0].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[0].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[1].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[1].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[1].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[2].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[2].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[2].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[3].y) * (-sin(SapphireEngine::DegToRad(ObjectRotation))), (RectPoints[3].x) * sin(SapphireEngine::DegToRad(ObjectRotation)) + (RectPoints[3].y) * cos(SapphireEngine::DegToRad(ObjectRotation)));

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

    RenderShape({
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, CamPos,CameraZoom,OutLine, WireFrame, Uniforms,Viewport);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}
