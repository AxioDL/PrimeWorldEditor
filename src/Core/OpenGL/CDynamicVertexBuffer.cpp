#include "CDynamicVertexBuffer.h"
#include "CVertexArrayManager.h"

#include <array>
#include <bit>

constexpr std::array<uint32_t, 12> gskAttribSize{
    0xC, 0xC, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8
};

CDynamicVertexBuffer::CDynamicVertexBuffer() = default;

CDynamicVertexBuffer::~CDynamicVertexBuffer()
{
    CVertexArrayManager::DeleteAllArraysForVBO(this);
    ClearBuffers();
}

void CDynamicVertexBuffer::SetVertexCount(uint32_t NumVerts)
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
    if (Attrib < EVertexAttribute::Position || Attrib > EVertexAttribute::Tex7)
        return;

    // Attribute values are power of 2, so we can use a CTZ to easily get the index.
    // e.g.
    // Pos -> 0b0001 | CTZ -> 0
    // Nor -> 0b0010 | CTZ -> 1
    // and so on.
    const auto Index = std::countr_zero(static_cast<uint32_t>(Attrib));
    glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[Index]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gskAttribSize[Index] * mNumVertices, pkData);
}

void CDynamicVertexBuffer::ClearBuffers()
{
    for (size_t iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const auto Bit = EVertexAttribute(1U << iAttrib);

        if (mBufferedFlags.HasFlag(Bit))
            glDeleteBuffers(1, &mAttribBuffers[iAttrib]);
    }

    mBufferedFlags.Reset(EVertexAttribute::None);
}

GLuint CDynamicVertexBuffer::CreateVAO()
{
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    for (uint32_t iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
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

    for (size_t iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
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
