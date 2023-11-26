#pragma once
#include "Utilities.hpp"
#include "Renderer.h"
#include "stb_image.h"


namespace SapphireRenderer{
    class Texture{
        public:
            static void SetTextureParameters();
            static const int& OccupyFreeSlot();
            static void FreeSlot(const int& SlotID);
            Texture(const std::string& Path, bool flip = false);
            Texture() {GLCall(glGenTextures(1, &ID));}
            ~Texture();
            void SetImage(int width, int height, unsigned char* Data);
            void Load(const std::string& Path, bool flip = false);
            void SetAsActive() const;
            void Init();
            void Bind() const;
            void Unbind() const;
            const int& GetSlot() const {return Slot;} 
            const GLuint& GetID() const {return ID;}
            const glm::vec2 GetDimensions() const {return glm::vec2(width, height);}
        private:
            int Slot = -1;
            GLuint ID = 0;
            unsigned int Size = 0;
            unsigned char* Data = nullptr;
            int width, height, nrChannels;
            static std::queue<int> FreeTextureSlots;
    };
}