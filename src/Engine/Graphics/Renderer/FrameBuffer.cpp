#include "FrameBuffer.h"
#include "Editor.h"
void SapphireRenderer::FrameBuffer::Init()
{
    glGenTextures(1, &FBOTexture.GetID());
    glBindTexture(GL_TEXTURE_2D, FBOTexture.GetID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    GLCall(glGenFramebuffers(1, &ID));
    FrameBuffer::CheckFBO();
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture.GetID(), 0);
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void SapphireRenderer::FrameBuffer::CheckFBO()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // Handle framebuffer incomplete error
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                printf("Framebuffer Error: Default framebuffer is not complete\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                printf("Framebuffer Error: One or more framebuffer attachments are not complete\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                printf("Framebuffer Error: No attachments were specified\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                printf("Framebuffer Error: Draw buffer attachment point is not complete\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                printf("Framebuffer Error: Read buffer attachment point is not complete\n");
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                printf("Framebuffer Error: Unsupported framebuffer format/configuration\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                printf("Framebuffer Error: Sample buffers do not have the same number of samples\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                printf("Framebuffer Error: Layered attachments do not have the same number of samples\n");
                break;
            default:
                printf("Framebuffer Error: Unknown error\n");
                break;
        }
    }
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
    // RendFrameBuffer.Bind();
    // RendFrameBuffer.GetTexture().Bind();
    // RendFrameBuffer.GetTexture().SetImage(width, height, nullptr);
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RendFrameBuffer.GetTexture().GetID(), 0));
    // RendFrameBuffer.Unbind();

    GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, ID));
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, RendFrameBuffer.GetID()));
    GLCall(glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::RescaleFrameBuffer(float width, float height)
{
    glBindTexture(GL_TEXTURE_2D, RendFrameBuffer.GetTexture().GetID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture));
	GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (int)(Editor::AASamples.Get()), GL_RGB, width, height, GL_TRUE));
    GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0));
}

void SapphireRenderer::MultisampleTextureFrameBuffer::Init()
{
    RendFrameBuffer.Init();

    glGenTextures(1, &texture);
    GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture));
	GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, (int)(Editor::AASamples.Get()), GL_RGB, 1280, 720, GL_TRUE));

    GLCall(glGenFramebuffers(1, &ID));
    FrameBuffer::CheckFBO();
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
