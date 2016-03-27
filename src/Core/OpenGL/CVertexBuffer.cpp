#include "CVertexBuffer.h"
#include "CVertexArrayManager.h"

CVertexBuffer::CVertexBuffer()
{
    mBuffered = false;
    SetVertexDesc(ePosition | eNormal | eTex0 | eTex1 | eTex2 | eTex3 | eTex4 | eTex5 | eTex6 | eTex7);
}

CVertexBuffer::CVertexBuffer(FVertexDescription Desc)
{
    mBuffered = false;
    SetVertexDesc(Desc);
}

CVertexBuffer::~CVertexBuffer()
{
    CVertexArrayManager::DeleteAllArraysForVBO(this);

    if (mBuffered)
        glDeleteBuffers(12, mAttribBuffers);
}

u16 CVertexBuffer::AddVertex(const CVertex& rkVtx)
{
    if (mPositions.size() == 0xFFFF) throw std::overflow_error("VBO contains too many vertices");

    if (mVtxDesc & ePosition) mPositions.push_back(rkVtx.Position);
    if (mVtxDesc & eNormal)   mNormals.push_back(rkVtx.Normal);
    if (mVtxDesc & eColor0)   mColors[0].push_back(rkVtx.Color[0]);
    if (mVtxDesc & eColor1)   mColors[1].push_back(rkVtx.Color[1]);

    for (u32 iTex = 0; iTex < 8; iTex++)
        if (mVtxDesc & (eTex0 << (iTex * 2))) mTexCoords[iTex].push_back(rkVtx.Tex[iTex]);

    for (u32 iMtx = 0; iMtx < 8; iMtx++)
        if (mVtxDesc & (ePosMtx << iMtx)) mTexCoords[iMtx].push_back(rkVtx.MatrixIndices[iMtx]);

    return (mPositions.size() - 1);
}

u16 CVertexBuffer::AddIfUnique(const CVertex& rkVtx, u16 Start)
{
    if (Start < mPositions.size())
    {
        for (u16 iVert = Start; iVert < mPositions.size(); iVert++)
        {
            // I use a bool because "continue" doesn't work properly within the iTex loop
            bool Unique = false;

            if (mVtxDesc & ePosition)
                if (rkVtx.Position != mPositions[iVert]) Unique = true;

            if ((!Unique) && (mVtxDesc & eNormal))
                if (rkVtx.Normal != mNormals[iVert]) Unique = true;

            if ((!Unique) && (mVtxDesc & eColor0))
                if (rkVtx.Color[0] != mColors[0][iVert]) Unique = true;

            if ((!Unique) && (mVtxDesc & eColor1))
                if (rkVtx.Color[1] != mColors[1][iVert]) Unique = true;

            if (!Unique)
                for (u32 iTex = 0; iTex < 8; iTex++)
                    if ((mVtxDesc & (eTex0 << (iTex * 2))))
                        if (rkVtx.Tex[iTex] != mTexCoords[iTex][iVert])
                        {
                            Unique = true;
                            break;
                        }

            if (!Unique) return iVert;
        }
    }

    return AddVertex(rkVtx);
}

void CVertexBuffer::Reserve(u16 Size)
{
    u32 ReserveSize = mPositions.size() + Size;

    if (mVtxDesc & ePosition)
        mPositions.reserve(ReserveSize);

    if (mVtxDesc & eNormal)
        mNormals.reserve(ReserveSize);

    if (mVtxDesc & eColor0)
        mColors[0].reserve(ReserveSize);

    if (mVtxDesc & eColor1)
        mColors[1].reserve(ReserveSize);

    for (u32 iTex = 0; iTex < 8; iTex++)
        if (mVtxDesc & (eTex0 << (iTex * 2)))
            mTexCoords[iTex].reserve(ReserveSize);
}

void CVertexBuffer::Clear()
{
    if (mBuffered)
        glDeleteBuffers(12, mAttribBuffers);

    mBuffered = false;
    mPositions.clear();
    mNormals.clear();
    mColors[0].clear();
    mColors[1].clear();

    for (u32 iTex = 0; iTex < 8; iTex++)
        mTexCoords[iTex].clear();
}

void CVertexBuffer::Buffer()
{
    // Make sure we don't end up with two buffers for the same data...
    if (mBuffered)
    {
        glDeleteBuffers(12, mAttribBuffers);
        mBuffered = false;
    }

    // Generate buffers
    glGenBuffers(12, mAttribBuffers);

    for (u32 iAttrib = 0; iAttrib < 12; iAttrib++)
    {
        int Attrib = (ePosition << (iAttrib * 2));
        bool HasAttrib = ((mVtxDesc & Attrib) != 0);
        if (!HasAttrib) continue;

        if (iAttrib < 2)
        {
            std::vector<CVector3f> *pBuffer = (iAttrib == 0) ? &mPositions : &mNormals;

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, pBuffer->size() * sizeof(CVector3f), pBuffer->data(), GL_STATIC_DRAW);
        }

        else if (iAttrib < 4)
        {
            u8 Index = (u8) (iAttrib - 2);

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mColors[Index].size() * sizeof(CColor), mColors[Index].data(), GL_STATIC_DRAW);
        }

        else
        {
            u8 Index = (u8) (iAttrib - 4);

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mTexCoords[Index].size() * sizeof(CVector2f), mTexCoords[Index].data(), GL_STATIC_DRAW);
        }
    }

    mBuffered = true;
}

void CVertexBuffer::Bind()
{
    if (!mBuffered) Buffer();
    CVertexArrayManager::Current()->BindVAO(this);
}

void CVertexBuffer::Unbind()
{
    glBindVertexArray(0);
}

bool CVertexBuffer::IsBuffered()
{
    return mBuffered;
}

FVertexDescription CVertexBuffer::VertexDesc()
{
    return mVtxDesc;
}

void CVertexBuffer::SetVertexDesc(FVertexDescription Desc)
{
    Clear();
    mVtxDesc = Desc;
}

u32 CVertexBuffer::Size()
{
    return mPositions.size();
}

GLuint CVertexBuffer::CreateVAO()
{
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    for (u32 iAttrib = 0; iAttrib < 12; iAttrib++)
    {
        int Attrib = (ePosition << (iAttrib * 2));
        bool HasAttrib = ((mVtxDesc & Attrib) != 0);
        if (!HasAttrib) continue;

        if (iAttrib < 2)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3f), (void*) 0);
            glEnableVertexAttribArray(iAttrib);
        }

        else if (iAttrib < 4)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(CColor), (void*) 0);
            glEnableVertexAttribArray(iAttrib);
        }

        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(CVector2f), (void*) 0);
            glEnableVertexAttribArray(iAttrib);
        }
    }

    glBindVertexArray(0);
    return VertexArray;
}
