#pragma once
#include "Utilities.hpp"
#include "Graphics/Renderer/FrameBuffer.h"
#include "Imgui/imgui_markdown.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui_stdlib.h"
#include "Imgui/imconfig.h"
#include "Imgui/ImGuizmo.h"

class Window{
    public:
        inline static std::vector<Window*> Windows;
        Window(const std::string& name, bool Default = true);
        virtual void Display() {};
        void Update();
        const bool& GetState() {return Active;}
        const std::string& GetName() { return Name;}
        void SetState(bool state) {Active = state;}
    protected:
        std::string Name;
        bool Active;
};

class WindowFBO : public Window{
    public:
        WindowFBO(const std::string& name);
        virtual void Draw() {};
        virtual void Extras() {};
        void Display() override;
        glm::vec2 GetDimensions() {return glm::vec2(width, height);}
    protected:
        SapphireRenderer::MultisampleTextureFrameBuffer fbo;
        int width, height;
        ImVec2 LastSize = ImVec2(0,0);
};

class LogWindow : public Window{
    public:
        LogWindow() : Window("Logs") {}
        void Display() override;
};
