#include "CRenderbuffer.h"

CRenderbuffer::CRenderbuffer()
{
    mInitialized = false;
    mWidth = 0;
    mHeight = 0;
}

CRenderbuffer::CRenderbuffer(u32 Width, u32 Height)
{
    mInitialized = false;
    mWidth = Width;
    mHeight = Height;
}

CRenderbuffer::~CRenderbuffer()
{
    if (mInitialized)
        glDeleteRenderbuffers(1, &mRenderbuffer);
}

void CRenderbuffer::Init()
{
    glGenRenderbuffers(1, &mRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    mInitialized = true;
}

void CRenderbuffer::Resize(u32 Width, u32 Height)
{
    mWidth = Width;
    mHeight = Height;

    if (mInitialized)
    {
        Bind();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    }
}

void CRenderbuffer::Bind()
{
    if (!mInitialized) Init();
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
}

void CRenderbuffer::Unbind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
