#include "CFramebuffer.h"
#include <Common/Log.h>

CFramebuffer::CFramebuffer()
    : mInitialized(false)
    , mWidth(0)
    , mHeight(0)
    , mpRenderbuffer(nullptr)
    , mpTexture(nullptr)
{
}

CFramebuffer::CFramebuffer(u32 Width, u32 Height)
    : mInitialized(false)
    , mWidth(0)
    , mHeight(0)
    , mpRenderbuffer(nullptr)
    , mpTexture(nullptr)
{
    Resize(Width, Height);
}

CFramebuffer::~CFramebuffer()
{
    if (mInitialized)
    {
        glDeleteFramebuffers(1, &mFramebuffer);
        delete mpRenderbuffer;
        delete mpTexture;
    }
}

void CFramebuffer::Init()
{
    if (!smStaticsInitialized)
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &smDefaultFramebuffer);
        smStaticsInitialized = true;
    }

    if (!mInitialized)
    {
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        mpRenderbuffer = new CRenderbuffer(mWidth, mHeight);
        mpRenderbuffer->Bind();
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mpRenderbuffer->BufferID()
        );

        mpTexture = new CTexture(mWidth, mHeight);
        mpTexture->Bind(0);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mpTexture->TextureID(), 0
        );

        mStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (mStatus != GL_FRAMEBUFFER_COMPLETE)
            Log::Error("Framebuffer not complete");

        mInitialized = true;
    }
}

void CFramebuffer::Bind()
{
    if (!mInitialized) Init();
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
}

void CFramebuffer::Resize(u32 Width, u32 Height)
{
    if ((mWidth != Width) || (mHeight != Height))
    {
        mWidth = Width;
        mHeight = Height;

        if (mInitialized)
        {
            mpRenderbuffer->Resize(Width, Height);
            mpTexture->Resize(Width, Height);

            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
            mpRenderbuffer->Bind();
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mpRenderbuffer->BufferID()
            );

            mpTexture->Bind(0);
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mpTexture->TextureID(), 0
            );
        }
    }
}

CTexture* CFramebuffer::Texture()
{
    return mpTexture;
}

// ************ STATIC ************
void CFramebuffer::BindDefaultFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, smDefaultFramebuffer);
}

GLint CFramebuffer::smDefaultFramebuffer;
bool CFramebuffer::smStaticsInitialized;
