#include "CFramebuffer.h"
#include <Common/Log.h>

CFramebuffer::CFramebuffer()
    : mpRenderbuffer(nullptr)
    , mpTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mEnableMultisampling(false)
    , mInitialized(false)
{
}

CFramebuffer::CFramebuffer(u32 Width, u32 Height)
    : mpRenderbuffer(nullptr)
    , mpTexture(nullptr)
    , mWidth(0)
    , mHeight(0)
    , mEnableMultisampling(false)
    , mInitialized(false)
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
        mpTexture = new CTexture(mWidth, mHeight);
        mpRenderbuffer->SetMultisamplingEnabled(mEnableMultisampling);
        mpTexture->SetMultisamplingEnabled(mEnableMultisampling);
        InitBuffers();
        mInitialized = true;
    }
}

void CFramebuffer::Bind(GLenum Target /*= GL_FRAMEBUFFER*/)
{
    if (!mInitialized) Init();
    glBindFramebuffer(Target, mFramebuffer);
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
            InitBuffers();
        }
    }
}

void CFramebuffer::SetMultisamplingEnabled(bool Enable)
{
    if (mEnableMultisampling != Enable)
    {
        mEnableMultisampling = Enable;

        if (mInitialized)
        {
            mpRenderbuffer->SetMultisamplingEnabled(Enable);
            mpTexture->SetMultisamplingEnabled(Enable);
            InitBuffers();
        }
    }
}

// ************ PROTECTED ************
void CFramebuffer::InitBuffers()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    mpRenderbuffer->Bind();
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mpRenderbuffer->BufferID()
    );

    mpTexture->Bind(0);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), mpTexture->TextureID(), 0
    );

    mStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (mStatus != GL_FRAMEBUFFER_COMPLETE)
        Log::Error("Framebuffer not complete; error " + TString::HexString((u32) mStatus, 0));
}

// ************ STATIC ************
void CFramebuffer::BindDefaultFramebuffer(GLenum Target /*= GL_FRAMEBUFFER*/)
{
    glBindFramebuffer(Target, smDefaultFramebuffer);
}

GLint CFramebuffer::smDefaultFramebuffer;
bool CFramebuffer::smStaticsInitialized;
