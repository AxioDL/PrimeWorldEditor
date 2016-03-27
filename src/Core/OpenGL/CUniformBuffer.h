#ifndef CUNIFORMBUFFER_H
#define CUNIFORMBUFFER_H

#include <Common/types.h>
#include <GL/glew.h>

class CUniformBuffer
{
    GLuint mUniformBuffer;
    u32 mBufferSize;

public:

    CUniformBuffer()
    {
        glGenBuffers(1, &mUniformBuffer);
        SetBufferSize(0);
    }

    CUniformBuffer(u32 Size)
    {
        glGenBuffers(1, &mUniformBuffer);
        SetBufferSize(Size);
    }

    ~CUniformBuffer()
    {
        glDeleteBuffers(1, &mUniformBuffer);
    }

    void Bind()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    }

    void Unbind()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void BindBase(GLuint Index)
    {
        Bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, Index, mUniformBuffer);
        Unbind();
    }

    void Buffer(void *pData)
    {
        Bind();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, mBufferSize, pData);
        Unbind();
    }

    void SetBufferSize(u32 Size)
    {
        mBufferSize = Size;
        InitializeBuffer();
    }

    u32 GetBufferSize()
    {
        return mBufferSize;
    }

private:
    void InitializeBuffer()
    {
        Bind();
        glBufferData(GL_UNIFORM_BUFFER, mBufferSize, 0, GL_DYNAMIC_DRAW);
        Unbind();
    }
};

#endif // CUNIFORMBUFFER_H
