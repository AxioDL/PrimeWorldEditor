#include "CModel.h"
#include <Core/CRenderer.h>
#include <OpenGL/GLCommon.h>

CModel::CModel() : CBasicModel()
{
    mHasOwnMaterials = true;
    mHasOwnSurfaces = true;
}

CModel::CModel(SModelData *pModelData) : CBasicModel()
{
    SetData(pModelData);
    mHasOwnMaterials = false;
    mHasOwnSurfaces = true;
}

CModel::CModel(SModelData *data, CMaterialSet *pMatSet) : CBasicModel()
{
    SetData(data);
    mMaterialSets.resize(1);
    mMaterialSets[0] = pMatSet;
    mHasOwnMaterials = false;
    mHasOwnSurfaces = true;
}

CModel::~CModel()
{
    if (mHasOwnMaterials)
        for (u32 m = 0; m < mMaterialSets.size(); m++)
            delete mMaterialSets[m];
}

void CModel::SetData(SModelData *pModelData)
{
    mAABox = pModelData->mAABox;
    mSurfaces = pModelData->mSurfaces;

    mVertexCount = 0;
    mTriangleCount = 0;

    for (u32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
    {
        SSurface *pSurf = mSurfaces[iSurf];
        mVertexCount += pSurf->VertexCount;
        mTriangleCount += pSurf->TriangleCount;
    }
}

void CModel::BufferGL()
{
    mVBO.Clear();
    mSubmeshIndexBuffers.clear();

    mSubmeshIndexBuffers.resize(mSurfaces.size());

    for (u32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
    {
        SSurface *pSurf = mSurfaces[iSurf];

        u16 VBOStartOffset = (u16) mVBO.Size();
        mVBO.Reserve((u16) pSurf->VertexCount);

        for (u32 iPrim = 0; iPrim < pSurf->Primitives.size(); iPrim++)
        {
            SSurface::SPrimitive *pPrim = &pSurf->Primitives[iPrim];
            CIndexBuffer *pIBO = InternalGetIBO(iSurf, pPrim->Type);
            pIBO->Reserve(pPrim->Vertices.size() + 1); // Allocate enough space for this primitive, plus the restart index

            std::vector<u16> Indices(pPrim->Vertices.size());
            for (u32 iVert = 0; iVert < pPrim->Vertices.size(); iVert++)
                Indices[iVert] = mVBO.AddIfUnique(pPrim->Vertices[iVert], VBOStartOffset);

            // then add the indices to the IBO. We convert some primitives to strips to minimize draw calls.
            switch (pPrim->Type) {
                case eGX_Triangles:
                    pIBO->TrianglesToStrips(Indices.data(), Indices.size());
                    break;
                case eGX_TriangleFan:
                    pIBO->FansToStrips(Indices.data(), Indices.size());
                    break;
                case eGX_Quads:
                    pIBO->QuadsToStrips(Indices.data(), Indices.size());
                    break;
                default:
                    pIBO->AddIndices(Indices.data(), Indices.size());
                    pIBO->AddIndex(0xFFFF); // primitive restart
                    break;
            }
        }
    }

    mBuffered = true;
}

void CModel::ClearGLBuffer()
{
    mVBO.Clear();
    mSubmeshIndexBuffers.clear();
    mBuffered = false;
}

void CModel::Draw(ERenderOptions Options, u32 MatSet)
{
    if (!mBuffered) BufferGL();
    for (u32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        DrawSurface(Options, iSurf, MatSet);
}

void CModel::DrawSurface(ERenderOptions Options, u32 Surface, u32 MatSet)
{
    if (!mBuffered) BufferGL();

    // Check that mat set index is valid
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    // Bind material
    SSurface *pSurf = mSurfaces[Surface];
    CMaterial *pMat = mMaterialSets[MatSet]->materials[pSurf->MaterialID];

    if ((Options & eNoMaterialSetup) == 0)
    {
        if ((!(Options & eEnableOccluders)) && (pMat->Options() & CMaterial::eOccluder))
            return;

        pMat->SetCurrent(Options);
    }

    // Draw IBOs
    mVBO.Bind();

    for (u32 iIBO = 0; iIBO < mSubmeshIndexBuffers[Surface].size(); iIBO++)
    {
        CIndexBuffer *pIBO = &mSubmeshIndexBuffers[Surface][iIBO];
        pIBO->DrawElements();
    }

    mVBO.Unbind();
}

u32 CModel::GetMatSetCount()
{
    return mMaterialSets.size();
}

u32 CModel::GetMatCount()
{
    if (mMaterialSets.empty()) return 0;
    else return mMaterialSets[0]->materials.size();
}

CMaterialSet* CModel::GetMatSet(u32 MatSet)
{
    return mMaterialSets[MatSet];
}

CMaterial* CModel::GetMaterialByIndex(u32 MatSet, u32 Index)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    if (GetMatCount() == 0)
        return nullptr;

    return mMaterialSets[MatSet]->materials[Index];
}

CMaterial* CModel::GetMaterialBySurface(u32 MatSet, u32 Surface)
{
    return GetMaterialByIndex(MatSet, mSurfaces[Surface]->MaterialID);
}

bool CModel::HasTransparency(u32 MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    for (u32 iMat = 0; iMat < mMaterialSets[MatSet]->materials.size(); iMat++)
        if (mMaterialSets[MatSet]->materials[iMat]->Options() & CMaterial::eTransparent ) return true;

    return false;
}

bool CModel::IsSurfaceTransparent(u32 Surface, u32 MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    u32 matID = mSurfaces[Surface]->MaterialID;
    return (mMaterialSets[MatSet]->materials[matID]->Options() & CMaterial::eTransparent) != 0;
}

CIndexBuffer* CModel::InternalGetIBO(u32 Surface, EGXPrimitiveType Primitive)
{
    std::vector<CIndexBuffer> *pIBOs = &mSubmeshIndexBuffers[Surface];
    GLenum Type = GXPrimToGLPrim(Primitive);

    for (u32 iIBO = 0; iIBO < pIBOs->size(); iIBO++)
    {
        if ((*pIBOs)[iIBO].GetPrimitiveType() == Type)
            return &(*pIBOs)[iIBO];
    }

    pIBOs->emplace_back(CIndexBuffer(Type));
    return &pIBOs->back();
}
