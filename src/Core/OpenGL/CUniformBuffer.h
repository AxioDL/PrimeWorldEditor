#ifndef CUNIFORMBUFFER_H
#define CUNIFORMBUFFER_H

#include <Common/types.h>
#include <GL/glew.h>

class CUniformBuffer
{
    GLuint mUniformBuffer;
    u32 mBufferSize;

public:
    CUniformBuffer();
    CUniformBuffer(u32 Size);
    ~CUniformBuffer();
    void Bind();
    void Unbind();
    void BindBase(GLuint index);
    void Buffer(void *pData);

    void SetBufferSize(u32 Size);
    u32 GetBufferSize();

private:
    void InitializeBuffer();
};

#endif // CUNIFORMBUFFER_H
