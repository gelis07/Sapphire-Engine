#pragma once
#include "Utilities.hpp"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

class Grid{
    public:
        ~Grid();
        void Init();
        void Render(const glm::vec3& CameraPos, float CameraZoom);
    private:
        SapphireRenderer::VertexArray* VertexArray = nullptr;
        SapphireRenderer::VertexBuffer* VertexBuffer = nullptr;
        SapphireRenderer::IndexBuffer* IndexBuffer = nullptr;
        SapphireRenderer::Shader* Shader = nullptr;
        glm::mat4 mvp;
};