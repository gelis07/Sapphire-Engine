#pragma once
#include "Windows.h"


template <typename T>
class DragAndDrop
{
    public:
        void StartedDragging(T NewData);
        T* ReceiveDrop(ImGuiWindow *window);
    private:
        T m_Data;
        bool m_Dragging;
};

template <typename T>
void DragAndDrop<T>::StartedDragging(T NewData)
{
    m_Data = NewData;
    m_Dragging = ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

template <typename T>
T *DragAndDrop<T>::ReceiveDrop(ImGuiWindow *window)
{
    double xpos, ypos;
    glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
    glm::vec2 CursorPos(xpos, ypos);
    glm::vec2 D(window->Pos.x, window->Pos.y); // Bottom left corner of window
    glm::vec2 A(window->Pos.x + window->Size.x, window->Pos.y + window->Size.y); // Top right corner
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_Dragging &&(D.x < CursorPos.x && D.y < CursorPos.y && A.x > CursorPos.x && A.y > CursorPos.y))
    {
        m_Dragging = false;
        return &m_Data; 
    }else if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_Dragging && !(D.x < CursorPos.x && D.y < CursorPos.y && A.x > CursorPos.x && A.y > CursorPos.y)){
        m_Dragging = false;
        return nullptr;
    }
    else{
        return nullptr;
    }
}
