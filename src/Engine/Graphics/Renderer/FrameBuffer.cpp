#include "FrameBuffer.h"

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
    FBOTexture.Bind();
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
}

void SapphireRenderer::FrameBuffer::Unbind() const
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    FBOTexture.Unbind();
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
