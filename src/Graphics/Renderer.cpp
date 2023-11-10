#include "Renderer.h"
void Renderer::Render(const Transform& transform, const glm::mat4& view, bool&& IsSelected ,glm::vec3 CameraPos,float CameraZoom)
{
    //That means that the object is an empty
    if(shape == nullptr) return;
    // Here it renders the object's outline to indicate that the current object is selected
    if(IsSelected){
        shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,true, shape->Wireframe());
    }
    shape->Render(transform, Color.Get(),CameraPos,view,CameraZoom,false, shape->Wireframe());
}

void Renderer::CustomRendering()
{
    if(TexturePath.Get() != ""){
        ImGui::Text(TexturePath.Get().c_str());
        std::stringstream ss;
        ss << "width: " << shape->GetTextureDimensions().x << ", height: " << shape->GetTextureDimensions().y;
        ImGui::Text(ss.str().c_str());
        if(ImGui::Button("Remove")){
            shape->DeleteTexture();
            TexturePath.Get() = "";
            shape->SetShader(SapphireRenderer::BasicShader);
        }
    }
}
