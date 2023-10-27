#include "Renderer.h"
void Renderer::Render(Transform& transform, bool&& IsSelected ,glm::vec3 CameraPos,float CameraZoom)
{
    //That means that the object is an empty
    if(shape == nullptr) return;
    // Here it renders the object's outline to indicate that the current object is selected
    if(IsSelected){
        shape->Render(transform, CameraPos ,CameraZoom,true, shape->Wireframe());
    }
    shape->Render(transform, CameraPos ,CameraZoom,false, shape->Wireframe());
}