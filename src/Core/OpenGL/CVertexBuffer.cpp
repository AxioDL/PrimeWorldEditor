#include "CVertexBuffer.h"
#include "CVertexArrayManager.h"

CVertexBuffer::CVertexBuffer()
{
    SetVertexDesc(EVertexAttribute::Position | EVertexAttribute::Normal |
                  EVertexAttribute::Tex0 | EVertexAttribute::Tex1 |
                  EVertexAttribute::Tex2 | EVertexAttribute::Tex3 |
                  EVertexAttribute::Tex4 | EVertexAttribute::Tex5 |
                  EVertexAttribute::Tex6 | EVertexAttribute::Tex7);
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
        glDeleteBuffers(static_cast<GLsizei>(mAttribBuffers.size()), mAttribBuffers.data());
}

uint16 CVertexBuffer::AddVertex(const CVertex& rkVtx)
{
    if (mPositions.size() == 0xFFFF)
        throw std::overflow_error("VBO contains too many vertices");

    if ((mVtxDesc & EVertexAttribute::Position) != 0)
        mPositions.emplace_back(rkVtx.Position);
    if ((mVtxDesc & EVertexAttribute::Normal) != 0)
        mNormals.emplace_back(rkVtx.Normal);
    if ((mVtxDesc & EVertexAttribute::Color0) != 0)
        mColors[0].emplace_back(rkVtx.Color[0]);
    if ((mVtxDesc & EVertexAttribute::Color1) != 0)
        mColors[1].emplace_back(rkVtx.Color[1]);

    for (size_t iTex = 0; iTex < mTexCoords.size(); iTex++)
    {
        if ((mVtxDesc & (EVertexAttribute::Tex0 << iTex)) != 0)
            mTexCoords[iTex].emplace_back(rkVtx.Tex[iTex]);
    }

    for (size_t iMtx = 0; iMtx < mTexCoords.size(); iMtx++)
    {
        if ((mVtxDesc & (EVertexAttribute::PosMtx << iMtx)) != 0)
            mTexCoords[iMtx].emplace_back(rkVtx.MatrixIndices[iMtx]);
    }

    if (mVtxDesc.HasAnyFlags(EVertexAttribute::BoneIndices | EVertexAttribute::BoneWeights) && mpSkin != nullptr)
    {
        const SVertexWeights& rkWeights = mpSkin->WeightsForVertex(rkVtx.ArrayPosition);
        if ((mVtxDesc & EVertexAttribute::BoneIndices) != 0)
            mBoneIndices.emplace_back(rkWeights.Indices);
        if ((mVtxDesc & EVertexAttribute::BoneWeights) != 0)
            mBoneWeights.emplace_back(rkWeights.Weights);
    }

    return mPositions.size() - 1;
}

uint16 CVertexBuffer::AddIfUnique(const CVertex& rkVtx, uint16 Start)
{
    if (Start < mPositions.size())
    {
        for (size_t iVert = Start; iVert < mPositions.size(); iVert++)
        {
            // I use a bool because "continue" doesn't work properly within the iTex loop
            bool Unique = false;

            if ((mVtxDesc & EVertexAttribute::Position) != 0)
            {
                if (rkVtx.Position != mPositions[iVert])
                    Unique = true;
            }

            if (!Unique && (mVtxDesc & EVertexAttribute::Normal) != 0)
            {
                if (rkVtx.Normal != mNormals[iVert])
                    Unique = true;
            }

            if (!Unique && (mVtxDesc & EVertexAttribute::Color0) != 0)
            {
                if (rkVtx.Color[0] != mColors[0][iVert])
                    Unique = true;
            }

            if (!Unique && (mVtxDesc & EVertexAttribute::Color1) != 0)
            {
                if (rkVtx.Color[1] != mColors[1][iVert])
                    Unique = true;
            }

            if (!Unique)
            {
                for (size_t iTex = 0; iTex < mTexCoords.size(); iTex++)
                {
                    if ((mVtxDesc & (EVertexAttribute::Tex0 << iTex)) != 0)
                    {
                        if (rkVtx.Tex[iTex] != mTexCoords[iTex][iVert])
                        {
                            Unique = true;
                            break;
                        }
                    }
                }
            }

            if (!Unique && mpSkin != nullptr && (mVtxDesc.HasAnyFlags(EVertexAttribute::BoneIndices | EVertexAttribute::BoneWeights)))
            {
                const SVertexWeights& rkWeights = mpSkin->WeightsForVertex(rkVtx.ArrayPosition);

                for (uint32 iWgt = 0; iWgt < 4; iWgt++)
                {
                    if (((mVtxDesc & EVertexAttribute::BoneIndices) != 0 && (rkWeights.Indices[iWgt] != mBoneIndices[iVert][iWgt])) ||
                        ((mVtxDesc & EVertexAttribute::BoneWeights) != 0 && (rkWeights.Weights[iWgt] != mBoneWeights[iVert][iWgt])))
                    {
                        Unique = true;
                        break;
                    }
                }
            }

            if (!Unique)
                return static_cast<uint16>(iVert);
        }
    }

    return AddVertex(rkVtx);
}

void CVertexBuffer::Reserve(size_t Size)
{
    const size_t ReserveSize = mPositions.size() + Size;

    if ((mVtxDesc & EVertexAttribute::Position) != 0)
        mPositions.reserve(ReserveSize);

    if ((mVtxDesc & EVertexAttribute::Normal) != 0)
        mNormals.reserve(ReserveSize);

    if ((mVtxDesc & EVertexAttribute::Color0) != 0)
        mColors[0].reserve(ReserveSize);

    if ((mVtxDesc & EVertexAttribute::Color1) != 0)
        mColors[1].reserve(ReserveSize);

    for (size_t iTex = 0; iTex < mTexCoords.size(); iTex++)
    {
        if ((mVtxDesc & (EVertexAttribute::Tex0 << iTex)) != 0)
            mTexCoords[iTex].reserve(ReserveSize);
    }

    if ((mVtxDesc & EVertexAttribute::BoneIndices) != 0)
        mBoneIndices.reserve(ReserveSize);

    if ((mVtxDesc & EVertexAttribute::BoneWeights) != 0)
        mBoneWeights.reserve(ReserveSize);
}

void CVertexBuffer::Clear()
{
    if (mBuffered)
        glDeleteBuffers(static_cast<GLsizei>(mAttribBuffers.size()), mAttribBuffers.data());

    mBuffered = false;
    mPositions.clear();
    mNormals.clear();
    mColors[0].clear();
    mColors[1].clear();

    for (auto& coord : mTexCoords)
        coord.clear();

    mBoneIndices.clear();
    mBoneWeights.clear();
}

void CVertexBuffer::Buffer()
{
    // Make sure we don't end up with two buffers for the same data...
    if (mBuffered)
    {
        glDeleteBuffers(static_cast<GLsizei>(mAttribBuffers.size()), mAttribBuffers.data());
        mBuffered = false;
    }

    // Generate buffers
    glGenBuffers(static_cast<GLsizei>(mAttribBuffers.size()), mAttribBuffers.data());

    for (size_t iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const auto Attrib = static_cast<int>(EVertexAttribute::Position << iAttrib);
        const bool HasAttrib = (mVtxDesc & Attrib) != 0;
        if (!HasAttrib)
            continue;

        if (iAttrib < 2)
        {
            const std::vector<CVector3f>& pBuffer = iAttrib == 0 ? mPositions : mNormals;

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, pBuffer.size() * sizeof(CVector3f), pBuffer.data(), GL_STATIC_DRAW);
        }
        else if (iAttrib < 4)
        {
            const auto Index = static_cast<uint8>(iAttrib - 2);

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mColors[Index].size() * sizeof(CColor), mColors[Index].data(), GL_STATIC_DRAW);
        }
        else if (iAttrib < 12)
        {
            const auto Index = static_cast<uint8>(iAttrib - 4);

            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mTexCoords[Index].size() * sizeof(CVector2f), mTexCoords[Index].data(), GL_STATIC_DRAW);
        }
        else if (iAttrib == 12)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mBoneIndices.size() * sizeof(TBoneIndices), mBoneIndices.data(), GL_STATIC_DRAW);
        }
        else if (iAttrib == 13)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glBufferData(GL_ARRAY_BUFFER, mBoneWeights.size() * sizeof(TBoneWeights), mBoneWeights.data(), GL_STATIC_DRAW);
        }
    }

    mBuffered = true;
}

void CVertexBuffer::Bind()
{
    if (!mBuffered)
        Buffer();

    CVertexArrayManager::Current()->BindVAO(this);
}

void CVertexBuffer::Unbind()
{
    glBindVertexArray(0);
}

bool CVertexBuffer::IsBuffered() const
{
    return mBuffered;
}

FVertexDescription CVertexBuffer::VertexDesc() const
{
    return mVtxDesc;
}

void CVertexBuffer::SetVertexDesc(FVertexDescription Desc)
{
    Clear();
    mVtxDesc = Desc;
}

void CVertexBuffer::SetSkin(CSkin *pSkin)
{
    Clear();
    mpSkin = pSkin;
}

size_t CVertexBuffer::Size() const
{
    return mPositions.size();
}

GLuint CVertexBuffer::CreateVAO()
{
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    for (size_t iAttrib = 0; iAttrib < mAttribBuffers.size(); iAttrib++)
    {
        const auto Attrib = static_cast<int>(EVertexAttribute::Position << iAttrib);
        const bool HasAttrib = (mVtxDesc & Attrib) != 0;
        if (!HasAttrib) continue;

        if (iAttrib < 2)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(CVector3f), nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
        else if (iAttrib < 4)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(CColor), nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
        else if (iAttrib < 12)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(CVector2f), nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
        else if (iAttrib == 12)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribIPointer(iAttrib, 1, GL_UNSIGNED_INT, sizeof(TBoneIndices), nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
        else if (iAttrib == 13)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mAttribBuffers[iAttrib]);
            glVertexAttribPointer(iAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(TBoneWeights), nullptr);
            glEnableVertexAttribArray(iAttrib);
        }
    }

    glBindVertexArray(0);
    return VertexArray;
}
