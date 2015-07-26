#include "CIndexBuffer.h"

CIndexBuffer::CIndexBuffer()
{
    mBuffered = false;
}

CIndexBuffer::CIndexBuffer(GLenum type)
{
    mPrimitiveType = type;
    mBuffered = false;
}

CIndexBuffer::~CIndexBuffer()
{
    if (mBuffered)
        glDeleteBuffers(1, &mIndexBuffer);
}

void CIndexBuffer::AddIndex(u16 idx)
{
    mIndices.push_back(idx);
}

void CIndexBuffer::AddIndices(u16 *indicesPtr, u32 count)
{
    Reserve(count);
    for (u32 i = 0; i < count; i++)
        mIndices.push_back(*indicesPtr++);
}

void CIndexBuffer::Reserve(u32 size)
{
    mIndices.reserve(mIndices.size() + size);
}

void CIndexBuffer::Clear()
{
    if (mBuffered)
        glDeleteBuffers(1, &mIndexBuffer);

    mBuffered = false;
    mIndices.clear();
}

void CIndexBuffer::Buffer()
{
    if (mBuffered)
        glDeleteBuffers(1, &mIndexBuffer);

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(u16), mIndices.data(), GL_STATIC_DRAW);

    mBuffered = true;
}

void CIndexBuffer::Bind()
{
    if (!mBuffered) Buffer();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
}

void CIndexBuffer::Unbind()
{
}

void CIndexBuffer::DrawElements()
{
    Bind();
    glDrawElements(mPrimitiveType, mIndices.size(), GL_UNSIGNED_SHORT, (void*) 0);
    Unbind();
}

void CIndexBuffer::DrawElements(u32 Offset, u32 Size)
{
    Bind();
    glDrawElements(mPrimitiveType, Size, GL_UNSIGNED_SHORT, (void*) (Offset * 2));
    Unbind();
}

bool CIndexBuffer::IsBuffered()
{
    return mBuffered;
}

u32 CIndexBuffer::GetSize()
{
    return mIndices.size();
}

GLenum CIndexBuffer::GetPrimitiveType()
{
    return mPrimitiveType;
}

void CIndexBuffer::SetPrimitiveType(GLenum type)
{
    mPrimitiveType = type;
}

void CIndexBuffer::TrianglesToStrips(u16 *indicesPtr, u32 count)
{
    Reserve(count + (count / 3));

    for (u32 i = 0; i < count; i += 3)
    {
        mIndices.push_back(*indicesPtr++);
        mIndices.push_back(*indicesPtr++);
        mIndices.push_back(*indicesPtr++);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::FansToStrips(u16 *indicesPtr, u32 count)
{
    Reserve(count);
    u16 FirstIndex = *indicesPtr;

    for (u32 i = 2; i < count; i += 3)
    {
        mIndices.push_back(indicesPtr[i - 1]);
        mIndices.push_back(indicesPtr[i]);
        mIndices.push_back(FirstIndex);
        if (i + 1 < count)
            mIndices.push_back(indicesPtr[i + 1]);
        if (i + 2 < count)
            mIndices.push_back(indicesPtr[i + 2]);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::QuadsToStrips(u16 *indicesPtr, u32 count)
{
    Reserve((u32) (count * 1.25));

    u32 i = 3;
    for (; i < count; i += 4)
    {
        mIndices.push_back(indicesPtr[i - 2]);
        mIndices.push_back(indicesPtr[i - 1]);
        mIndices.push_back(indicesPtr[i - 3]);
        mIndices.push_back(indicesPtr[i]);
        mIndices.push_back(0xFFFF);
    }

    // if there's three indices present that indicates a single triangle
    if (i == count)
    {
        mIndices.push_back(indicesPtr[i - 3]);
        mIndices.push_back(indicesPtr[i - 2]);
        mIndices.push_back(indicesPtr[i - 1]);
        mIndices.push_back(0xFFFF);
    }

}
