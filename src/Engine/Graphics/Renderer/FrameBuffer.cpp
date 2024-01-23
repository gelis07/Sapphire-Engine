#include "FrameBuffer.h"
#include "Editor.h"
void SapphireRenderer::FrameBuffer::Init()
{
    FBOTexture.Init();
    GLCall(glGenFramebuffers(1, &ID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture.GetID(), 0);
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

SapphireRenderer::FrameBuffer::~FrameBuffer()
{
    GLCall(glDeleteFramebuffers(1, &ID));
}

void SapphireRenderer::FrameBuffer::Bind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
}

void SapphireRenderer::FrameBuffer::Unbind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void SapphireRenderer::FrameBuffer::RescaleFrameBuffer(float width, float height)
{
	FBOTexture.Bind();
	FBOTexture.SetImage(width, height, NULL);
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture.GetID(), 0);
	FBOTexture.Unbind();
}

SapphireRenderer::MultisampleTextureFrameBuffer::~MultisampleTextureFrameBuffer()
{
    GLCall(glDeleteFramebuffers(1, &ID));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::Blit(float width, float height)
{
    RendFrameBuffer.Bind();
    RendFrameBuffer.GetTexture().Bind();
    RendFrameBuffer.GetTexture().SetImage(width, height, nullptr);
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RendFrameBuffer.GetTexture().GetID(), 0));
    RendFrameBuffer.Unbind();

    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, ID));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, RendFrameBuffer.GetID()));
    GLCall(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::RescaleFrameBuffer(float width, float height)
{
    GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture));
	GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (int)(Editor::AASamples.Get()), GL_RGB, width, height, GL_TRUE));
    GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::Init()
{
    RendFrameBuffer.Init();

    GLCall(glGenTextures(1, &texture));
    GLCall(glGenFramebuffers(1, &ID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::Bind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::Unbind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
