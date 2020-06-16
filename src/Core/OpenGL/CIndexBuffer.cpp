#include "CIndexBuffer.h"

CIndexBuffer::CIndexBuffer() = default;

CIndexBuffer::CIndexBuffer(GLenum type)
    : mPrimitiveType(type)
{
}

CIndexBuffer::~CIndexBuffer()
{
    if (mBuffered)
        glDeleteBuffers(1, &mIndexBuffer);
}

void CIndexBuffer::AddIndex(uint16 index)
{
    mIndices.push_back(index);
}

void CIndexBuffer::AddIndices(const uint16 *indices, size_t count)
{
    Reserve(count);
    for (size_t i = 0; i < count; i++)
        mIndices.push_back(*indices++);
}

void CIndexBuffer::Reserve(size_t size)
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(uint16), mIndices.data(), GL_STATIC_DRAW);

    mBuffered = true;
}

void CIndexBuffer::Bind()
{
    if (!mBuffered)
        Buffer();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
}

void CIndexBuffer::Unbind()
{
}

void CIndexBuffer::DrawElements()
{
    Bind();
    glDrawElements(mPrimitiveType, mIndices.size(), GL_UNSIGNED_SHORT, nullptr);
    Unbind();
}

void CIndexBuffer::DrawElements(uint offset, uint size)
{
    Bind();
    glDrawElements(mPrimitiveType, size, GL_UNSIGNED_SHORT, (char*)0 + (offset * 2));
    Unbind();
}

bool CIndexBuffer::IsBuffered() const
{
    return mBuffered;
}

uint CIndexBuffer::GetSize() const
{
    return mIndices.size();
}

GLenum CIndexBuffer::GetPrimitiveType() const
{
    return mPrimitiveType;
}

void CIndexBuffer::SetPrimitiveType(GLenum type)
{
    mPrimitiveType = type;
}

void CIndexBuffer::TrianglesToStrips(uint16 *indices, size_t count)
{
    Reserve(count + (count / 3));

    for (size_t i = 0; i < count; i += 3)
    {
        mIndices.push_back(*indices++);
        mIndices.push_back(*indices++);
        mIndices.push_back(*indices++);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::FansToStrips(uint16 *indices, size_t count)
{
    Reserve(count);
    const uint16 firstIndex = *indices;

    for (size_t i = 2; i < count; i += 3)
    {
        mIndices.push_back(indices[i - 1]);
        mIndices.push_back(indices[i]);
        mIndices.push_back(firstIndex);
        if (i + 1 < count)
            mIndices.push_back(indices[i + 1]);
        if (i + 2 < count)
            mIndices.push_back(indices[i + 2]);
        mIndices.push_back(0xFFFF);
    }
}

void CIndexBuffer::QuadsToStrips(uint16 *indices, size_t count)
{
    Reserve(static_cast<size_t>(count * 1.25));

    size_t i = 3;
    for (; i < count; i += 4)
    {
        mIndices.push_back(indices[i - 2]);
        mIndices.push_back(indices[i - 1]);
        mIndices.push_back(indices[i - 3]);
        mIndices.push_back(indices[i]);
        mIndices.push_back(0xFFFF);
    }

    // if there's three indices present that indicates a single triangle
    if (i == count)
    {
        mIndices.push_back(indices[i - 3]);
        mIndices.push_back(indices[i - 2]);
        mIndices.push_back(indices[i - 1]);
        mIndices.push_back(0xFFFF);
    }

}
