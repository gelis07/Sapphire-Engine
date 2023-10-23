#include "Texture.h"

void SapphireRenderer::Texture::SetTextureParameters()
{
    stbi_set_flip_vertically_on_load(false);
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
}

SapphireRenderer::Texture::Texture(const std::string &Path)
{
    GLCall(glGenTextures(1, &ID));
    GLCall(glBindTexture(GL_TEXTURE_2D, ID));
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
}
// SapphireRenderer::Texture::Texture()
// {
//     GLCall(glGenTextures(1, &ID));
//     Bind();
//     GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
//     Unbind();
// }

SapphireRenderer::Texture::~Texture()
{
    stbi_image_free(Data);
    // GLCall(glDeleteTextures(1, &ID));
}

void SapphireRenderer::Texture::SetImage(int aWidth, int aHeight, unsigned char* aData)
{
    width = aWidth;
    height = aHeight;
    Data = aData;
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
}

void SapphireRenderer::Texture::Load(const std::string &Path)
{
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
    // GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    Unbind();
}

void SapphireRenderer::Texture::Init()
{
    GLCall(glGenTextures(1, &ID));
}

void SapphireRenderer::Texture::Bind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, ID));
}

void SapphireRenderer::Texture::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
