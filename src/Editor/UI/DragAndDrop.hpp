#pragma once


template <typename T>
class DragAndDrop
{
    public:
        void StartedDragging(T NewData);
        T* ReceiveDrop(ImGuiWindow *window);
        T* ReceiveDrop(const glm::vec2& pos, const glm::vec2& size);
        T* ReceiveDropLoop(const glm::vec2& pos, const glm::vec2& size);
        void CalcDragging();
        const bool& IsDragging() {return m_Dragging;}
    private:
        bool FinishedDragging = false;
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
    ImVec2 CursorPos = ImGui::GetMousePos();
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
template <typename T>
T *DragAndDrop<T>::ReceiveDrop(const glm::vec2& pos,const glm::vec2& size)
{
    ImVec2 CursorPos = ImGui::GetMousePos();
    glm::vec2 D(pos.x, pos.y); // Bottom left corner of window
    glm::vec2 A(pos.x + size.x, pos.y + size.y); // Top right corner
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

template <typename T>
inline T *DragAndDrop<T>::ReceiveDropLoop(const glm::vec2 &pos, const glm::vec2 &size)
{
    ImVec2 CursorPos = ImGui::GetMousePos();
    glm::vec2 D(pos.x, pos.y); // Bottom left corner of window
    glm::vec2 A(pos.x + size.x, pos.y + size.y); // Top right corner
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_Dragging &&(D.x < CursorPos.x && D.y < CursorPos.y && A.x > CursorPos.x && A.y > CursorPos.y))
    {
        m_Dragging = false;
        return &m_Data; 
    }else if(!ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_Dragging && !(D.x < CursorPos.x && D.y < CursorPos.y && A.x > CursorPos.x && A.y > CursorPos.y)){
        FinishedDragging = true;
        return nullptr;
    }
    else{
        return nullptr;
    }
}

template <typename T>
inline void DragAndDrop<T>::CalcDragging()
{
    if(FinishedDragging){
        FinishedDragging = false;
        m_Dragging = false;
    }
}
