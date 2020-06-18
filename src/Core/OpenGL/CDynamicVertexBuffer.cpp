#include "CDynamicVertexBuffer.h"
#include "CVertexArrayManager.h"

#include <array>

constexpr std::array<uint32, 12> gskAttribSize{
    0xC, 0xC, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8
};

CDynamicVertexBuffer::CDynamicVertexBuffer() = default;

CDynamicVertexBuffer::~CDynamicVertexBuffer()
{
    CVertexArrayManager::DeleteAllArraysForVBO(this);
    ClearBuffers();
}

void CDynamicVertexBuffer::SetVertexCount(uint32 NumVerts)
{
    ClearBuffers();
    mNumVertices = NumVerts;
    InitBuffers();
}

void CDynamicVertexBuffer::Bind()
{
    CVertexArrayManager::Current()->BindVAO(this);
}

void CDynamicVertexBuffer::Unbind()
{
    glBindVertexArray(0);
}

void CDynamicVertexBuffer::SetActiveAttribs(FVertexDescription AttribFlags)
{
    ClearBuffers();
    mAttribFlags = AttribFlags;
    InitBuffers();
}

void CDynamicVertexBuffer::BufferAttrib(EVertexAttribute Attrib, const void *pkData)
{
    size_t Index;

    switch (Attrib)
    {
    case EVertexAttribute::Position:    Index = 0;  break;
    case EVertexAttribute::Normal:      Index = 1;  break;
    case EVertexAttribute::Color0:      Index = 2;  break;
    case EVertexAttribute::Color1:      Index = 3;  break;
    case EVertexAttribute::Tex0:        Index = 4;  break;
    case EVertexAttribute::Tex1:        Index = 5;  break;
    case EVertexAttribute::Tex2:        Index = 6;  break;
    case EVertexAttribute::Tex3:        Index = 7;  break;
    case EVertexAttribute::Tex4:        Index = 8;  break;
    case EVertexAttribute::Tex5:        Index = 9;  break;
    case EVertexAttribute::Tex6:        Index = 10; break;
    case EVertexAttribute::Tex7:        Index = 11; break;
    default:                            return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[Index]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gskAttribSize[Index] * mNumVertices, pkData);
}

void CDynamicVertexBuffer::ClearBuffers()
{
    for (uint32 iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const int Bit = 1 << iAttrib;

        if (mBufferedFlags & Bit)
            glDeleteBuffers(1, &mAttribBuffers[iAttrib]);
    }

    mBufferedFlags = EVertexAttribute::None;
}

GLuint CDynamicVertexBuffer::CreateVAO()
{
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    for (uint32 iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const bool HasAttrib = (3 << (iAttrib * 2)) != 0;

        if (HasAttrib)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            GLuint NumComponents;
            GLenum DataType;

            if (iAttrib == 2 || iAttrib == 3)
            {
                NumComponents = 4;
                DataType = GL_UNSIGNED_BYTE;
            }
            else
            {
                NumComponents = gskAttribSize[iAttrib] / 4;
                DataType = GL_FLOAT;
            }

            glVertexAttribPointer(iAttrib, NumComponents, DataType, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
    }

    glBindVertexArray(0);
    return VertexArray;
}

// ************ PRIVATE ************
void CDynamicVertexBuffer::InitBuffers()
{
    if (mBufferedFlags)
        ClearBuffers();

    for (uint32 iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const bool HasAttrib = ((3 << (iAttrib * 2)) != 0);

        if (HasAttrib)
        {
            glGenBuffers(1, &mAttribBuffers[iAttrib]);
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, gskAttribSize[iAttrib] * mNumVertices, nullptr, GL_DYNAMIC_DRAW);
        }
    }
    mBufferedFlags = mAttribFlags;
}
