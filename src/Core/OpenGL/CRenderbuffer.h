#ifndef CRENDERBUFFER_H
#define CRENDERBUFFER_H

#include <Common/types.h>
#include <GL/glew.h>

class CRenderbuffer
{
    GLuint mRenderbuffer;
    u32 mWidth, mHeight;
    bool mInitialized;

public:
    CRenderbuffer::CRenderbuffer()
        : mInitialized(false)
        , mWidth(0)
        , mHeight(0)
    {
    }

    CRenderbuffer::CRenderbuffer(u32 Width, u32 Height)
        : mInitialized(false)
        , mWidth(Width)
        , mHeight(Height)
    {
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

    inline void CRenderbuffer::Resize(u32 Width, u32 Height)
    {
        mWidth = Width;
        mHeight = Height;

        if (mInitialized)
        {
            Bind();
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
        }
    }

    inline void CRenderbuffer::Bind()
    {
        if (!mInitialized) Init();
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    }

    inline void CRenderbuffer::Unbind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    inline GLuint BufferID()
    {
        return mRenderbuffer;
    }
};

#endif // CRENDERBUFFER_H
