#ifndef CRENDERBUFFER_H
#define CRENDERBUFFER_H

#include <Common/BasicTypes.h>
#include <GL/glew.h>

class CRenderbuffer
{
    GLuint mRenderbuffer = 0;
    uint mWidth = 0;
    uint mHeight = 0;
    bool mEnableMultisampling = false;
    bool mInitialized = false;

public:
    CRenderbuffer() = default;
    CRenderbuffer(uint Width, uint Height)
        : mWidth(Width)
        , mHeight(Height)
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

    void Resize(uint Width, uint Height)
    {
        mWidth = Width;
        mHeight = Height;

        if (mInitialized)
            InitStorage();
    }

    void Bind()
    {
        if (!mInitialized) Init();
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    }

    void Unbind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    GLuint BufferID() const
    {
        return mRenderbuffer;
    }

    void SetMultisamplingEnabled(bool Enable)
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
