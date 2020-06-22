#include "CModel.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/OpenGL/GLCommon.h"
#include <Common/Macros.h>

CModel::CModel(CResourceEntry *pEntry)
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
    if (!mHasOwnMaterials)
        return;

    for (auto* set : mMaterialSets)
        delete set;
}


std::unique_ptr<CDependencyTree> CModel::BuildDependencyTree() const
{
    auto pTree = std::make_unique<CDependencyTree>();
    std::set<CAssetID> TextureIDs;

    for (CMaterialSet* set : mMaterialSets)
    {
        set->GetUsedTextureIDs(TextureIDs);
    }

    for (const auto& id : TextureIDs)
        pTree->AddDependency(id);

    return pTree;
}

void CModel::BufferGL()
{
    if (!mBuffered)
    {
        mVBO.Clear();
        mSurfaceIndexBuffers.clear();

        mSurfaceIndexBuffers.resize(mSurfaces.size());

        for (size_t iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        {
            SSurface *pSurf = mSurfaces[iSurf];

            uint16 VBOStartOffset = (uint16) mVBO.Size();
            mVBO.Reserve((uint16) pSurf->VertexCount);

            for (SSurface::SPrimitive& pPrim : pSurf->Primitives)
            {
                CIndexBuffer *pIBO = InternalGetIBO(iSurf, pPrim.Type);
                pIBO->Reserve(pPrim.Vertices.size() + 1); // Allocate enough space for this primitive, plus the restart index

                std::vector<uint16> Indices(pPrim.Vertices.size());
                for (size_t iVert = 0; iVert < pPrim.Vertices.size(); iVert++)
                    Indices[iVert] = mVBO.AddIfUnique(pPrim.Vertices[iVert], VBOStartOffset);

                // then add the indices to the IBO. We convert some primitives to strips to minimize draw calls.
                switch (pPrim.Type)
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

            for (auto& ibo : mSurfaceIndexBuffers[iSurf])
                ibo.Buffer();
        }

        mBuffered = true;
    }
}

void CModel::GenerateMaterialShaders()
{
    for (CMaterialSet* set : mMaterialSets)
    {
        for (size_t i = 0; i < set->NumMaterials(); i++)
        {
            set->MaterialByIndex(i, false)->GenerateShader(false);
            set->MaterialByIndex(i, true)->GenerateShader(false);
        }
    }
}

void CModel::ClearGLBuffer()
{
    mVBO.Clear();
    mSurfaceIndexBuffers.clear();
    mBuffered = false;
}

void CModel::Draw(FRenderOptions Options, size_t MatSet)
{
    if (!mBuffered)
        BufferGL();

    for (size_t iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
        DrawSurface(Options, iSurf, MatSet);
}

void CModel::DrawSurface(FRenderOptions Options, size_t Surface, size_t MatSet)
{
    if (!mBuffered)
        BufferGL();

    // Check that mat set index is valid
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    auto DoDraw = [this, Surface]()
    {
        // Draw IBOs
        mVBO.Bind();
        glLineWidth(1.f);

        for (auto& ibo : mSurfaceIndexBuffers[Surface])
        {
            ibo.DrawElements();
        }

        mVBO.Unbind();
    };

    // Bind material
    if ((Options & ERenderOption::NoMaterialSetup) == 0)
    {
        const SSurface *pSurf = mSurfaces[Surface];
        CMaterial *pMat = mMaterialSets[MatSet]->MaterialByIndex(pSurf->MaterialID, Options.HasFlag(ERenderOption::EnableBloom));

        if (!Options.HasFlag(ERenderOption::EnableOccluders) && pMat->Options().HasFlag(EMaterialOption::Occluder))
            return;

        for (CMaterial* passMat = pMat; passMat; passMat = passMat->GetNextDrawPass())
        {
            passMat->SetCurrent(Options);
            DoDraw();
        }
    }
    else
    {
        DoDraw();
    }
}

void CModel::DrawWireframe(FRenderOptions Options, CColor WireColor)
{
    if (!mBuffered)
        BufferGL();

    // Set up wireframe
    WireColor.A = 0;
    CDrawUtil::UseColorShader(WireColor);
    Options |= ERenderOption::NoMaterialSetup;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBlendFunc(GL_ONE, GL_ZERO);

    // Draw surfaces
    for (size_t iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
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

        for (CMaterialSet* set : mMaterialSets)
        {
            for (size_t iMat = 0; iMat < set->NumMaterials(); iMat++)
            {
                for (bool iBloom = false; !iBloom; iBloom = true)
                {
                    CMaterial *pMat = set->MaterialByIndex(iMat, iBloom);
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
}

size_t CModel::GetMatSetCount() const
{
    return mMaterialSets.size();
}

size_t CModel::GetMatCount() const
{
    if (mMaterialSets.empty())
        return 0;

    return mMaterialSets[0]->NumMaterials();
}

CMaterialSet* CModel::GetMatSet(size_t MatSet)
{
    return mMaterialSets[MatSet];
}

CMaterial* CModel::GetMaterialByIndex(size_t MatSet, size_t Index)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    if (GetMatCount() == 0)
        return nullptr;

    return mMaterialSets[MatSet]->MaterialByIndex(Index, false);
}

CMaterial* CModel::GetMaterialBySurface(size_t MatSet, size_t Surface)
{
    return GetMaterialByIndex(MatSet, mSurfaces[Surface]->MaterialID);
}

bool CModel::HasTransparency(size_t MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    for (size_t iMat = 0; iMat < mMaterialSets[MatSet]->NumMaterials(); iMat++)
    {
        if (mMaterialSets[MatSet]->MaterialByIndex(iMat, true)->Options() & EMaterialOption::Transparent)
            return true;
    }

    return false;
}

bool CModel::IsSurfaceTransparent(size_t Surface, size_t MatSet)
{
    if (MatSet >= mMaterialSets.size())
        MatSet = mMaterialSets.size() - 1;

    const uint32 matID = mSurfaces[Surface]->MaterialID;
    return (mMaterialSets[MatSet]->MaterialByIndex(matID, true)->Options() & EMaterialOption::Transparent) != 0;
}

bool CModel::IsLightmapped() const
{
    for (CMaterialSet* set : mMaterialSets)
    {
        for (size_t iMat = 0; iMat < set->NumMaterials(); iMat++)
        {
            const CMaterial *pMat = set->MaterialByIndex(iMat, true);
            if (pMat->Options().HasFlag(EMaterialOption::Lightmap))
                return true;
        }
    }
    return false;
}

CIndexBuffer* CModel::InternalGetIBO(size_t Surface, EPrimitiveType Primitive)
{
    std::vector<CIndexBuffer>& pIBOs = mSurfaceIndexBuffers[Surface];
    const GLenum Type = GXPrimToGLPrim(Primitive);

    for (auto& ibo : pIBOs)
    {
        if (ibo.GetPrimitiveType() == Type)
            return &ibo;
    }

    return &pIBOs.emplace_back(Type);
}
