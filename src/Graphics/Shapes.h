#pragma once
#include "Utilities.hpp"
#include <GL/glew.h>
#include "stb_image.h"
struct Vertex{
    float Pos[2];
};
struct TextureAtlas{
    glm::vec2 IconSize;
    glm::vec2 AtlasSize;
    GLuint AtlasID;
};

struct Camera{
    float Zoom = 1;
    glm::vec3 position;
};


static unsigned int CreateFBO(unsigned int texture){
    unsigned int fbo;
    GLCall(glGenFramebuffers(1, &fbo));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo)); 
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0)); 
    return fbo;
}

static unsigned int CreateViewportTexture()
{
    unsigned int texture;
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));
    
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); 
    return texture;
}
static unsigned int LoadTexture(std::string FilePath)
{
    int width, height, channels;
    unsigned char* image = stbi_load(FilePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    return textureID;
}
namespace Shapes
{
    inline Camera ViewportCamera; //This Camera is only for the viewport NOT the game's camera
    inline unsigned int BasicShader;
    inline unsigned int CircleShader;
    inline unsigned int GridShader;
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Shape{
        private:
            unsigned int VertexBuffer = 1;
            unsigned int VertexArray = 1;
            unsigned int IndexBuffer = 1;
            unsigned int shader = 1;
            glm::mat4 proj;
            glm::vec2 size;
        public:
            Shapes::Type ShapeType = Shapes::Null;
            inline void SetType(Shapes::Type argType) { ShapeType = argType;}
            void SetSize(glm::vec2 &ArgSize) {size = ArgSize;}
            void Construct(unsigned int sh); // Basically a virtual constructor
            virtual void SetUniforms(unsigned int shader, glm::mat4 &mvp, glm::vec2 &size, glm::vec2 startPoint, bool Viewport) {}
            // Thats the function that actually render's a shape
            void RenderShape(std::vector<Vertex> vertices,glm::vec4 c ,const glm::vec3 &pos,const glm::vec3 &CamPos ,bool WireFrame, bool ActualWireFrame = false, bool Viewport = true); 
            // Here is a virtual constructor for every sub class to do it's own calculations before passing in the data on RenderShape()
            virtual void Render(glm::vec2 Size, glm::vec4 Color,const glm::vec3 pos, float angle, const glm::vec3 &CamPos,bool WireFrame, bool ActualWireFrame = false, bool Viewport = true) {} 
    };

    class Rectangle : public Shape
    {  
        
        public:
            Rectangle(unsigned int sh);
            void SetUniforms(unsigned int shader, glm::mat4 &mvp, glm::vec2 &size, glm::vec2 startPoint, bool Viewport) override {
                GLCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
            }
            void Render(glm::vec2 Size, glm::vec4 Color, const glm::vec3 pos, float angle,const glm::vec3 &CamPos ,bool WireFrame, bool ActualWireFrame = false, bool Viewport = true) override;
    };
    class Circle : public Shape
    {
        public:
            Circle(unsigned int sh);
            void SetUniforms(unsigned int shader, glm::mat4 &mvp, glm::vec2 &size, glm::vec2 startPoint, bool IsViewport) override {
                GLCall(glUniformMatrix4fv(glGetUniformLocation(shader, "u_MVP"), 1,GL_FALSE, &mvp[0][0]));
                GLCall(glUniform1f(glGetUniformLocation(shader, "RectWidth"), size.x));
                GLCall(glUniform1f(glGetUniformLocation(shader, "RectHeight"), size.y));
                GLCall(glUniform2f(glGetUniformLocation(shader, "StartPoint"), startPoint.x, startPoint.y));
                if(IsViewport){
                    GLCall(glUniform1f(glGetUniformLocation(shader, "CameraZoom"), Shapes::ViewportCamera.Zoom));
                }else{
                    GLCall(glUniform1f(glGetUniformLocation(shader, "CameraZoom"), 1));
                }
            }
            void Render(glm::vec2 Size, glm::vec4 Color, const glm::vec3 pos, float angle, const glm::vec3 &CamPos,bool WireFrame, bool ActualWireFrame = false, bool Viewport = true) override;
    };

}

