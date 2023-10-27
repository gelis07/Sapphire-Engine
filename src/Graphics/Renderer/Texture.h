#pragma once
#include "Utilities.hpp"
#include "Renderer.h"
#include "stb_image.h"


namespace SapphireRenderer{
    class Texture{
        public:
            static void SetTextureParameters();
            Texture(const std::string& Path);
            Texture() {}
            ~Texture();
            void SetImage(int width, int height, unsigned char* Data);
            void Load(const std::string& Path);
            void Init();
            void Bind() const;
            void Unbind() const;

            const GLuint& GetID() const {return ID;}
            const glm::vec2 GetDimensions() const {return glm::vec2(width, height);}
        private:
            GLuint ID = 0;
            unsigned char* Data;
            int width, height, nrChannels;
    };
}