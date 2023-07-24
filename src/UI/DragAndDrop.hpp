#pragma once
#include "Windows.h"
#include "Engine/Views.h"


template <typename T>
class DragAndDrop
{
    public:
        void StartedDragging(T NewData);
        T* ReceiveDrop(ImGuiWindow *window);
    private:
        T Data;
        bool Dragging;
};

template <typename T>
void DragAndDrop<T>::StartedDragging(T NewData)
{
    Data = NewData;
    Dragging = ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

template <typename T>
T *DragAndDrop<T>::ReceiveDrop(ImGuiWindow *window)
{
    double xpos, ypos;
    glfwGetCursorPos(Viewport::window, &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    glm::vec2 D(window->Pos.x, window->Pos.y); // Bottom left corner of window
    glm::vec2 A(window->Pos.x + window->Size.x, window->Pos.y + window->Size.y); // Top right corner
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && Dragging &&(D.x < CursorPos.x && D.y < CursorPos.y && A.x > CursorPos.x && A.y > CursorPos.y))
    {
        Dragging = false;
        return &Data; 
    }else{
        return nullptr;
    }
}
