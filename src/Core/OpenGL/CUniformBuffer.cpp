#include "CUniformBuffer.h"

CUniformBuffer::CUniformBuffer()
{
    glGenBuffers(1, &mUniformBuffer);
    SetBufferSize(0);
}

CUniformBuffer::CUniformBuffer(u32 Size)
{
    glGenBuffers(1, &mUniformBuffer);
    SetBufferSize(Size);
}

CUniformBuffer::~CUniformBuffer()
{
    glDeleteBuffers(1, &mUniformBuffer);
}

void CUniformBuffer::InitializeBuffer()
{
    Bind();
    glBufferData(GL_UNIFORM_BUFFER, mBufferSize, 0, GL_DYNAMIC_DRAW);
    Unbind();
}

void CUniformBuffer::Bind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
}

void CUniformBuffer::Unbind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CUniformBuffer::BindBase(GLuint index)
{
    Bind();
    glBindBufferBase(GL_UNIFORM_BUFFER, index, mUniformBuffer);
    Unbind();
}

void CUniformBuffer::Buffer(void *pData)
{
    Bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, mBufferSize, pData);
    Unbind();
}

void CUniformBuffer::SetBufferSize(u32 Size)
{
    mBufferSize = Size;
    InitializeBuffer();
}

u32 CUniformBuffer::GetBufferSize()
{
    return mBufferSize;
}
