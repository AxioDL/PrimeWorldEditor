#ifndef CRENDERBUFFER_H
#define CRENDERBUFFER_H

#include <Common/BasicTypes.h>
#include <GL/glew.h>

class CRenderbuffer
{
    GLuint mRenderbuffer;
    uint mWidth, mHeight;
    bool mEnableMultisampling;
    bool mInitialized;

public:
    CRenderbuffer()
        : mWidth(0)
        , mHeight(0)
        , mEnableMultisampling(false)
        , mInitialized(false)
    {
    }

    CRenderbuffer(uint Width, uint Height)
        : mWidth(Width)
        , mHeight(Height)
        , mEnableMultisampling(false)
        , mInitialized(false)
    {
    }

    ~CRenderbuffer()
    {
        if (mInitialized)
            glDeleteRenderbuffers(1, &mRenderbuffer);
    }

    void Init()
    {
        mInitialized = true;
        glGenRenderbuffers(1, &mRenderbuffer);
        InitStorage();
    }

    inline void Resize(uint Width, uint Height)
    {
        mWidth = Width;
        mHeight = Height;

        if (mInitialized)
            InitStorage();
    }

    inline void Bind()
    {
        if (!mInitialized) Init();
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    }

    inline void Unbind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    inline GLuint BufferID()
    {
        return mRenderbuffer;
    }

    inline void SetMultisamplingEnabled(bool Enable)
    {
        if (mEnableMultisampling != Enable)
        {
            mEnableMultisampling = Enable;
            InitStorage();
        }
    }

private:
    void InitStorage()
    {
        Bind();

        if (mEnableMultisampling)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, mWidth, mHeight);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    }
};

#endif // CRENDERBUFFER_H
