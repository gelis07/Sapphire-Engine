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
Renderer::~Renderer(){
    shape.reset();
}