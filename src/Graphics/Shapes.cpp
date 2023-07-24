#include "Shapes.h"
#include <iostream>
#include <fstream>

static glm::vec4 LineColor(1.0f, 0.0f, 0.0f, 1.0f);
float lineWidth = 5.0f;
void Shapes::Shape::Construct(unsigned int sh) 
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

void Shapes::Shape::RenderShape(std::vector<Vertex> vertices,glm::vec4 c,const glm::vec3 &pos, const glm::vec3 &CamPos,bool WireFrame, bool ActualWireFrame, bool Viewport)
{
    proj = glm::ortho(0.0f, Viewport ? SCREEN_WIDTH / Shapes::ViewportCamera.Zoom : SCREEN_WIDTH, 0.0f, Viewport ? SCREEN_HEIGHT / Shapes::ViewportCamera.Zoom : SCREEN_HEIGHT, -1.0f, 1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
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
    glm::mat4 mvp = proj * view * model;
    glm::vec2 size(vertices[0].Pos[0] * 2, vertices[0].Pos[1] * 2);
    glm::vec2 StartPos(pos.x - size.x/2 + CamPos.x, pos.y - size.y/2 + CamPos.y);

    SetUniforms(shader, mvp,size, StartPos, Viewport);
    if(WireFrame){
        GLCall(glUniform4f(glGetUniformLocation(shader, "u_Color"), LineColor.r, LineColor.g, LineColor.b, LineColor.a));
    }else{
        GLCall(glUniform4f(glGetUniformLocation(shader, "u_Color"), c.r, c.g, c.b, c.a));
    }
    if(!ActualWireFrame){
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

Shapes::Rectangle::Rectangle(unsigned int sh)
{
    SetType(Shapes::RectangleT);
    Construct(sh);
}

Shapes::Circle::Circle(unsigned int sh)
{
    SetType(Shapes::CircleT);
    Construct(sh);
}
void Shapes::Rectangle::Render(glm::vec2 Size, glm::vec4 Color, const glm::vec3 pos, float angle,const glm::vec3 &CamPos ,bool WireFrame, bool ActualWireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2(Size.x/2, Size.y/2);
    RectPoints[1] = glm::vec2(-Size.x/2, Size.y/2);
    RectPoints[2] = glm::vec2(Size.x/2, -Size.y/2);
    RectPoints[3] = glm::vec2(-Size.x/2, -Size.y/2);

    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[0].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[0].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[0].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[1].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[1].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[1].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[2].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[2].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[2].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[3].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[3].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[3].y) * cos(Utilities::DegToRad(angle)));

    RenderShape({
                {NewRectPoints[3].x , NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, Color, pos, CamPos,WireFrame, ActualWireFrame, Viewport);
}

void Shapes::Circle::Render(glm::vec2 Size, glm::vec4 Color, const glm::vec3 pos, float angle,const glm::vec3 &CamPos ,bool WireFrame, bool ActualWireFrame, bool Viewport){
    std::array<glm::vec2, 4> RectPoints;
    //Getting each point of the rectangle
    RectPoints[0] = glm::vec2(Size.x/2, Size.y/2);
    RectPoints[1] = glm::vec2(-Size.x/2, Size.y/2);
    RectPoints[2] = glm::vec2(Size.x/2, -Size.y/2);
    RectPoints[3] = glm::vec2(-Size.x/2, -Size.y/2);

    //Here I'm using the standard rotation matrix https://en.wikipedia.org/wiki/Rotation_matrix
    std::array<glm::vec2, 4> NewRectPoints;
    NewRectPoints[0] = glm::vec2((RectPoints[0].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[0].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[0].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[0].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[1] = glm::vec2((RectPoints[1].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[1].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[1].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[1].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[2] = glm::vec2((RectPoints[2].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[2].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[2].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[2].y) * cos(Utilities::DegToRad(angle)));
    NewRectPoints[3] = glm::vec2((RectPoints[3].x) * cos(Utilities::DegToRad(angle)) + (RectPoints[3].y) * (-sin(Utilities::DegToRad(angle))), (RectPoints[3].x) * sin(Utilities::DegToRad(angle)) + (RectPoints[3].y) * cos(Utilities::DegToRad(angle)));

    RenderShape({
                {NewRectPoints[3].x, NewRectPoints[3].y},
                {NewRectPoints[2].x, NewRectPoints[2].y},
                {NewRectPoints[0].x, NewRectPoints[0].y},
               {NewRectPoints[1].x, NewRectPoints[1].y}
            }, Color, pos, CamPos ,WireFrame, ActualWireFrame, Viewport);

    /*  Now you might be thinking why I'm using a square to draw a circle but what I'm doing is 
        on the shader, I'm testing for each pixel if the vector from the square's origin to the pixel
        coordinates has a length less than the square's width/2. To study the code just head to Shaders/Circle.glsl .*/
}
