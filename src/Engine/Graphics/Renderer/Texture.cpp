#include "Texture.h"

std::queue<int> SapphireRenderer::Texture::FreeTextureSlots = {};

void SapphireRenderer::Texture::SetTextureParameters()
{
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    std::cout << "Available texture slots: " << maxTextureUnits << '\n';
    for (size_t i = 0; i <= maxTextureUnits; i++)
    {
        FreeTextureSlots.push(i);
    }
    
}

const int &SapphireRenderer::Texture::OccupyFreeSlot()
{
    FreeTextureSlots.pop();
    return FreeTextureSlots.front();
}

void SapphireRenderer::Texture::FreeSlot(const int &SlotID)
{
    if(SlotID < 0) return;
    FreeTextureSlots.push(SlotID);
}

SapphireRenderer::Texture::Texture(const std::string &Path, bool flip)
{
    GLCall(glGenTextures(1, &ID));
    Slot = OccupyFreeSlot();
    GLCall(glActiveTexture(GL_TEXTURE0 + Slot));
    GLCall(glBindTexture(GL_TEXTURE_2D, ID));
    stbi_set_flip_vertically_on_load(flip);
    Data = stbi_load(Path.c_str(), &width, &height, &nrChannels, 0);
    if (Data == nullptr) {
        width = 1;
        height = 1;
        nrChannels = 4;
        Data = new unsigned char[4];
        Data[0] = 255;
        Data[1] = 0;  
        Data[2] = 255;
        Data[3] = 255;
    }
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    stbi_image_free(Data);
}
SapphireRenderer::Texture::~Texture()
{
    FreeSlot(Slot);
    Unbind();
    if(ID != 0) GLCall(glDeleteTextures(1, &ID));
}
void SapphireRenderer::Texture::SetImage(int aWidth, int aHeight, unsigned char* aData)
{
    width = aWidth;
    height = aHeight;
    Data = aData;
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
}

void SapphireRenderer::Texture::Load(const std::string &Path, bool flip)
{
    if(Slot == -1) Slot = OccupyFreeSlot();
    GLCall(glActiveTexture(GL_TEXTURE0 + Slot));
    stbi_set_flip_vertically_on_load(flip);
    Bind();
    Data = stbi_load(Path.c_str(), &width, &height, &nrChannels, 0);
    if (Data == nullptr) {
        width = 1;
        height = 1;
        nrChannels = 4;
        Data = new unsigned char[4];
        Data[0] = 255;
        Data[1] = 0;  
        Data[2] = 255;
        Data[3] = 255;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data));
    GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    Unbind();
    stbi_image_free(Data);
}

void SapphireRenderer::Texture::SetAsActive() const
{
    GLCall(glActiveTexture(GL_TEXTURE0 + Slot))
    // GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data));
}

void SapphireRenderer::Texture::Init()
{
    GLCall(glGenTextures(1, &ID));
}

void SapphireRenderer::Texture::Bind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, ID));
    // std::cout << Data << '\n';
}

void SapphireRenderer::Texture::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
