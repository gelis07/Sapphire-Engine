#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include "Utilities.hpp"
#include "stb_image.h"


//The shapes class needs a refrence to an object because it neeeds to access its position and scale
class Object;


struct Vertex{
    float Pos[2];
};
struct TextureAtlas{
    glm::vec2 IconSize;
    glm::vec2 AtlasSize;
    GLuint AtlasID;
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

    unsigned int textureID;
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
    inline unsigned int BasicShader;
    inline unsigned int CircleShader;
    inline unsigned int GridShader;
    enum Type{
        RectangleT=1,CircleT=2,Null=-1
    };
    class Shape{
        public:
            Shapes::Type ShapeType = Shapes::Null;
            bool& Wireframe() {return m_Wireframe;}
            Shape(unsigned int Shader, std::shared_ptr<Object> NewObj);

            // That's the function that actually render's a shape
            void RenderShape(std::vector<Vertex> vertices, const glm::vec3 &CamPos, float CameraZoom,bool OutLine ,bool WireFrame, std::function<void(unsigned int shader)> SetUpUniforms,bool Viewport = true);

            // Here is a virtual Render() function for every sub class to do it's own calculations before passing in the data on RenderShape()
            virtual void Render(const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) {} 
        protected:
            std::shared_ptr<Object> m_ObjectRefrence;
        private:
            unsigned int m_Shader  = 1;
            unsigned int m_VertexBuffer = 1;
            unsigned int m_VertexArray = 1;
            unsigned int m_IndexBuffer = 1;
            glm::mat4 m_Projection;
            bool m_Wireframe = false;
    };

    class Rectangle : public Shape
    {
        public:
            Rectangle(unsigned int sh, std::shared_ptr<Object> NewObj) : Shape(sh, NewObj) {ShapeType = RectangleT;}
            void Render(const glm::vec3 &CamPos ,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    class Circle : public Shape
    {
        public:
            Circle(unsigned int sh, std::shared_ptr<Object> NewObj) : Shape(sh, NewObj) {ShapeType = CircleT;}
            void Render(const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    //To display the camera gizmo like portion of the screen.
    class CameraGizmo : public Shape
    {
        public:
            CameraGizmo(unsigned int sh, std::shared_ptr<Object> NewObj) : Shape(sh, NewObj) {ShapeType = RectangleT;}
            void Render(const glm::vec3 &CamPos,float CameraZoom,bool OutLine, bool WireFrame = false, bool Viewport = true) override;
    };
    

}

