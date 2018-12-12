#include "CIndexBuffer.h"

CIndexBuffer::CIndexBuffer()
    : mBuffered(false)
{
}

CIndexBuffer::CIndexBuffer(GLenum Type)
    : mPrimitiveType(Type)
    , mBuffered(false)
{
}

CIndexBuffer::~CIndexBuffer()
{
    if (mBuffered)
        glDeleteBuffers(1, &mIndexBuffer);
}

void CIndexBuffer::AddIndex(uint16 Index)
{
    mIndices.push_back(Index);
}

void CIndexBuffer::AddIndices(uint16 *pIndices, uint Count)
{
    Reserve(Count);
    for (uint iIdx = 0; iIdx < Count; iIdx++)
        mIndices.push_back(*pIndices++);
}

void CIndexBuffer::Reserve(uint Size)
{
    mIndices.reserve(mIndices.size() + Size);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(uint16), mIndices.data(), GL_STATIC_DRAW);

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

void CIndexBuffer::DrawElements(uint Offset, uint Size)
{
    Bind();
    glDrawElements(mPrimitiveType, Size, GL_UNSIGNED_SHORT, (char*)0 + (Offset * 2));
    Unbind();
}

bool CIndexBuffer::IsBuffered()
{
    return mBuffered;
}

uint CIndexBuffer::GetSize()
{
    return mIndices.size();
}

GLenum CIndexBuffer::GetPrimitiveType()
{
    return mPrimitiveType;
}

void CIndexBuffer::SetPrimitiveType(GLenum Type)
{
    mPrimitiveType = Type;
}

void CIndexBuffer::TrianglesToStrips(uint16 *pIndices, uint Count)
{
    Reserve(Count + (Count / 3));

    for (uint iIdx = 0; iIdx < Count; iIdx += 3)
    {
        mIndices.push_back(*pIndices++);
        mIndices.push_back(*pIndices++);
        mIndices.push_back(*pIndices++);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::FansToStrips(uint16 *pIndices, uint Count)
{
    Reserve(Count);
    uint16 FirstIndex = *pIndices;

    for (uint iIdx = 2; iIdx < Count; iIdx += 3)
    {
        mIndices.push_back(pIndices[iIdx - 1]);
        mIndices.push_back(pIndices[iIdx]);
        mIndices.push_back(FirstIndex);
        if (iIdx + 1 < Count)
            mIndices.push_back(pIndices[iIdx + 1]);
        if (iIdx + 2 < Count)
            mIndices.push_back(pIndices[iIdx + 2]);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::QuadsToStrips(uint16 *pIndices, uint Count)
{
    Reserve((uint) (Count * 1.25));

    uint iIdx = 3;
    for (; iIdx < Count; iIdx += 4)
    {
        mIndices.push_back(pIndices[iIdx - 2]);
        mIndices.push_back(pIndices[iIdx - 1]);
        mIndices.push_back(pIndices[iIdx - 3]);
        mIndices.push_back(pIndices[iIdx]);
        mIndices.push_back(0xFFFF);
    }

    // if there's three indices present that indicates a single triangle
    if (iIdx == Count)
    {
        mIndices.push_back(pIndices[iIdx - 3]);
        mIndices.push_back(pIndices[iIdx - 2]);
        mIndices.push_back(pIndices[iIdx - 1]);
        mIndices.push_back(0xFFFF);
    }

}
