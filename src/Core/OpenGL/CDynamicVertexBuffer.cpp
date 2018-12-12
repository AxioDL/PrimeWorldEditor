#include "CDynamicVertexBuffer.h"
#include "CVertexArrayManager.h"

static const uint32 gskAttribSize[] = {
    0xC, 0xC, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8
};

CDynamicVertexBuffer::CDynamicVertexBuffer()
    : mAttribFlags(eNoAttributes)
    , mBufferedFlags(eNoAttributes)
    , mNumVertices(0)
{
}

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
    uint32 Index;

    switch (Attrib)
    {
    case ePosition: Index = 0;  break;
    case eNormal:   Index = 1;  break;
    case eColor0:   Index = 2;  break;
    case eColor1:   Index = 3;  break;
    case eTex0:     Index = 4;  break;
    case eTex1:     Index = 5;  break;
    case eTex2:     Index = 6;  break;
    case eTex3:     Index = 7;  break;
    case eTex4:     Index = 8;  break;
    case eTex5:     Index = 9;  break;
    case eTex6:     Index = 10; break;
    case eTex7:     Index = 11; break;
    default: return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[Index]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gskAttribSize[Index] * mNumVertices, pkData);
}

void CDynamicVertexBuffer::ClearBuffers()
{
    for (uint32 iAttrib = 0; iAttrib < 12; iAttrib++)
    {
        int Bit = 1 << iAttrib;

        if (mBufferedFlags & Bit)
            glDeleteBuffers(1, &mAttribBuffers[iAttrib]);
    }

    mBufferedFlags = eNoAttributes;
}

GLuint CDynamicVertexBuffer::CreateVAO()
{
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    for (uint32 iAttrib = 0; iAttrib < 12; iAttrib++)
    {
        bool HasAttrib = ((3 << (iAttrib * 2)) != 0);

        if (HasAttrib)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            GLuint NumComponents;
            GLenum DataType;

            if ((iAttrib == 2) || (iAttrib == 3))
            {
                NumComponents = 4;
                DataType = GL_UNSIGNED_BYTE;
            }
            else
            {
                NumComponents = gskAttribSize[iAttrib] / 4;
                DataType = GL_FLOAT;
            }

            glVertexAttribPointer(iAttrib, NumComponents, DataType, GL_FALSE, 0, (void*) 0);
            glEnableVertexAttribArray(iAttrib);
        }
    }

    glBindVertexArray(0);
    return VertexArray;
}

// ************ PRIVATE ************
void CDynamicVertexBuffer::InitBuffers()
{
    if (mBufferedFlags) ClearBuffers();

    for (uint32 iAttrib = 0; iAttrib < 12; iAttrib++)
    {
        bool HasAttrib = ((3 << (iAttrib * 2)) != 0);

        if (HasAttrib)
        {
            glGenBuffers(1, &mAttribBuffers[iAttrib]);
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, gskAttribSize[iAttrib] * mNumVertices, NULL, GL_DYNAMIC_DRAW);
        }
    }
    mBufferedFlags = mAttribFlags;
}
