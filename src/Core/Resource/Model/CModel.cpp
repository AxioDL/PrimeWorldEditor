#include "CModel.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/OpenGL/GLCommon.h"
#include <Common/Macros.h>

CModel::CModel(CResourceEntry *pEntry /*= 0*/)
    : CBasicModel(pEntry)
{
    mHasOwnMaterials = true;
    mHasOwnSurfaces = true;
}

CModel::CModel(CMaterialSet *pSet, bool OwnsMatSet)
    : CBasicModel()
{
    mHasOwnMaterials = OwnsMatSet;
    mHasOwnSurfaces = true;

    mMaterialSets.resize(1);
    mMaterialSets[0] = pSet;
}

CModel::~CModel()
{
    if (mHasOwnMaterials)
        for (uint32 iMat = 0; iMat < mMaterialSets.size(); iMat++)
            delete mMaterialSets[iMat];
}


CDependencyTree* CModel::BuildDependencyTree() const
{
    CDependencyTree *pTree = new CDependencyTree();
    std::set<CAssetID> TextureIDs;

    for (uint32 iSet = 0; iSet < mMaterialSets.size(); iSet++)
    {
        CMaterialSet *pSet = mMaterialSets[iSet];
        pSet->GetUsedTextureIDs(TextureIDs);
    }

    for (auto Iter = TextureIDs.begin(); Iter != TextureIDs.end(); Iter++)
        pTree->AddDependency(*Iter);

    return pTree;
}

void CModel::BufferGL()
{
    if (!mBuffered)
    {
        mVBO.Clear();
        mSurfaceIndexBuffers.clear();

        mSurfaceIndexBuffers.resize(mSurfaces.size());

        for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        {
            SSurface *pSurf = mSurfaces[iSurf];

            uint16 VBOStartOffset = (uint16) mVBO.Size();
            mVBO.Reserve((uint16) pSurf->VertexCount);

            for (uint32 iPrim = 0; iPrim < pSurf->Primitives.size(); iPrim++)
            {
                SSurface::SPrimitive *pPrim = &pSurf->Primitives[iPrim];
                CIndexBuffer *pIBO = InternalGetIBO(iSurf, pPrim->Type);
                pIBO->Reserve(pPrim->Vertices.size() + 1); // Allocate enough space for this primitive, plus the restart index

                std::vector<uint16> Indices(pPrim->Vertices.size());
                for (uint32 iVert = 0; iVert < pPrim->Vertices.size(); iVert++)
                    Indices[iVert] = mVBO.AddIfUnique(pPrim->Vertices[iVert], VBOStartOffset);

                // then add the indices to the IBO. We convert some primitives to strips to minimize draw calls.
                switch (pPrim->Type)
                {
                    case EPrimitiveType::Triangles:
                        pIBO->TrianglesToStrips(Indices.data(), Indices.size());
                        break;
                    case EPrimitiveType::TriangleFan:
                        pIBO->FansToStrips(Indices.data(), Indices.size());
                        break;
                    case EPrimitiveType::Quads:
                        pIBO->QuadsToStrips(Indices.data(), Indices.size());
                        break;
                    default:
                        pIBO->AddIndices(Indices.data(), Indices.size());
                        pIBO->AddIndex(0xFFFF); // primitive restart
                        break;
                }
            }

            for (uint32 iIBO = 0; iIBO < mSurfaceIndexBuffers[iSurf].size(); iIBO++)
                mSurfaceIndexBuffers[iSurf][iIBO].Buffer();
        }

        mBuffered = true;
    }
}

void CModel::GenerateMaterialShaders()
{
    for (uint32 iSet = 0; iSet < mMaterialSets.size(); iSet++)
    {
        CMaterialSet *pSet = mMaterialSets[iSet];

        for (uint32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
        {
            CMaterial *pMat = pSet->MaterialByIndex(iMat);
            pMat->GenerateShader(false);
        }
    }
}

void CModel::ClearGLBuffer()
{
    mVBO.Clear();
    mSurfaceIndexBuffers.clear();
    mBuffered = false;
}

void CModel::Draw(FRenderOptions Options, uint32 MatSet)
{
    if (!mBuffered) BufferGL();
    for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        DrawSurface(Options, iSurf, MatSet);
}

void CModel::DrawSurface(FRenderOptions Options, uint32 Surface, uint32 MatSet)
{
    if (!mBuffered) BufferGL();

    // Check that mat set index is valid
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    // Bind material
    if ((Options & ERenderOption::NoMaterialSetup) == 0)
    {
        SSurface *pSurf = mSurfaces[Surface];
        CMaterial *pMat = mMaterialSets[MatSet]->MaterialByIndex(pSurf->MaterialID);

        if (!Options.HasFlag(ERenderOption::EnableOccluders) && pMat->Options().HasFlag(EMaterialOption::Occluder))
            return;

        pMat->SetCurrent(Options);
    }

    // Draw IBOs
    mVBO.Bind();
    glLineWidth(1.f);

    for (uint32 iIBO = 0; iIBO < mSurfaceIndexBuffers[Surface].size(); iIBO++)
    {
        CIndexBuffer *pIBO = &mSurfaceIndexBuffers[Surface][iIBO];
        pIBO->DrawElements();
    }

    mVBO.Unbind();
}

void CModel::DrawWireframe(FRenderOptions Options, CColor WireColor /*= CColor::skWhite*/)
{
    if (!mBuffered) BufferGL();

    // Set up wireframe
    WireColor.A = 0;
    CDrawUtil::UseColorShader(WireColor);
    Options |= ERenderOption::NoMaterialSetup;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBlendFunc(GL_ONE, GL_ZERO);

    // Draw surfaces
    for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        DrawSurface(Options, iSurf, 0);

    // Cleanup
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CModel::SetSkin(CSkin *pSkin)
{
    // Assert commented out because it actually failed somewhere! Needs to be addressed.
    //ASSERT(!mpSkin || !pSkin || mpSkin == pSkin); // This is to verify no model has more than one unique skin applied

    //@todo this is actually really dumb and could be handled much better (and much more inline with how the game does it)
    // by simply storing skinning data in a different vertex buffer that isn't tied to the model's vertex buffer
    if (mpSkin != pSkin)
    {
        const FVertexDescription kBoneFlags = (EVertexAttribute::BoneIndices | EVertexAttribute::BoneWeights);

        mpSkin = pSkin;
        mVBO.SetSkin(pSkin);
        ClearGLBuffer();

        if (pSkin && !mVBO.VertexDesc().HasAllFlags(kBoneFlags))
            mVBO.SetVertexDesc(mVBO.VertexDesc() | kBoneFlags);
        else if (!pSkin && mVBO.VertexDesc().HasAnyFlags(kBoneFlags))
            mVBO.SetVertexDesc(mVBO.VertexDesc() & ~kBoneFlags);

        for (uint32 iSet = 0; iSet < mMaterialSets.size(); iSet++)
        {
            CMaterialSet *pSet = mMaterialSets[iSet];

            for (uint32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
            {
                CMaterial *pMat = pSet->MaterialByIndex(iMat);
                FVertexDescription VtxDesc = pMat->VtxDesc();

                if (pSkin && !VtxDesc.HasAllFlags(kBoneFlags))
                {
                    VtxDesc |= kBoneFlags;
                    pMat->SetVertexDescription(VtxDesc);
                }

                else if (!pSkin && VtxDesc.HasAnyFlags(kBoneFlags))
                {
                    VtxDesc &= ~kBoneFlags;
                    pMat->SetVertexDescription(VtxDesc);
                }
            }
        }
    }
}

uint32 CModel::GetMatSetCount()
{
    return mMaterialSets.size();
}

uint32 CModel::GetMatCount()
{
    if (mMaterialSets.empty()) return 0;
    else return mMaterialSets[0]->NumMaterials();
}

CMaterialSet* CModel::GetMatSet(uint32 MatSet)
{
    return mMaterialSets[MatSet];
}

CMaterial* CModel::GetMaterialByIndex(uint32 MatSet, uint32 Index)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    if (GetMatCount() == 0)
        return nullptr;

    return mMaterialSets[MatSet]->MaterialByIndex(Index);
}

CMaterial* CModel::GetMaterialBySurface(uint32 MatSet, uint32 Surface)
{
    return GetMaterialByIndex(MatSet, mSurfaces[Surface]->MaterialID);
}

bool CModel::HasTransparency(uint32 MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    for (uint32 iMat = 0; iMat < mMaterialSets[MatSet]->NumMaterials(); iMat++)
        if (mMaterialSets[MatSet]->MaterialByIndex(iMat)->Options() & EMaterialOption::Transparent ) return true;

    return false;
}

bool CModel::IsSurfaceTransparent(uint32 Surface, uint32 MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    uint32 matID = mSurfaces[Surface]->MaterialID;
    return (mMaterialSets[MatSet]->MaterialByIndex(matID)->Options() & EMaterialOption::Transparent) != 0;
}

bool CModel::IsLightmapped() const
{
    for (uint32 iSet = 0; iSet < mMaterialSets.size(); iSet++)
    {
        CMaterialSet *pSet = mMaterialSets[iSet];

        for (uint32 iMat = 0; iMat < pSet->NumMaterials(); iMat++)
        {
            CMaterial *pMat = pSet->MaterialByIndex(iMat);
            if (pMat->Options().HasFlag(EMaterialOption::Lightmap))
                return true;
        }
    }
    return false;
}

CIndexBuffer* CModel::InternalGetIBO(uint32 Surface, EPrimitiveType Primitive)
{
    std::vector<CIndexBuffer> *pIBOs = &mSurfaceIndexBuffers[Surface];
    GLenum Type = GXPrimToGLPrim(Primitive);

    for (uint32 iIBO = 0; iIBO < pIBOs->size(); iIBO++)
    {
        if ((*pIBOs)[iIBO].GetPrimitiveType() == Type)
            return &(*pIBOs)[iIBO];
    }

    pIBOs->emplace_back(CIndexBuffer(Type));
    return &pIBOs->back();
}
