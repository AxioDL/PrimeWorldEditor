#include "CSceneNode.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Resource/CGameArea.h"
#include "Core/Resource/CResCache.h"
#include <Math/CTransform4f.h>

#include <algorithm>

u32 CSceneNode::smNumNodes = 0;
CColor CSceneNode::skSelectionTint = CColor::Integral(39, 154, 167);

CSceneNode::CSceneNode(CScene *pScene, u32 NodeID, CSceneNode *pParent)
    : mpScene(pScene)
    , mpParent(pParent)
    , _mID(NodeID)
    , mPosition(CVector3f::skZero)
    , mRotation(CQuaternion::skIdentity)
    , mScale(CVector3f::skOne)
    , _mTransformDirty(true)
    , _mInheritsPosition(true)
    , _mInheritsRotation(true)
    , _mInheritsScale(true)
    , mLightLayerIndex(0)
    , mLightCount(0)
    , mMouseHovering(false)
    , mSelected(false)
    , mVisible(true)
{
    smNumNodes++;

    if (mpParent)
        mpParent->mChildren.push_back(this);
}

CSceneNode::~CSceneNode()
{
    smNumNodes--;
    for (auto it = mChildren.begin(); it != mChildren.end(); it++)
        delete (*it);
}

// ************ VIRTUAL ************
void CSceneNode::DrawSelection()
{
    // Default implementation for virtual function
    CDrawUtil::DrawWireCube(AABox(), CColor::skWhite);
}

void CSceneNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    // Default implementation for virtual function
    std::pair<bool,float> Result = AABox().IntersectsRay(rTester.Ray());

    if (Result.first)
        rTester.AddNode(this, -1, Result.second);
}

bool CSceneNode::IsVisible() const
{
    // Default implementation for virtual function
    return mVisible;
}

CColor CSceneNode::TintColor(const SViewInfo& rkViewInfo) const
{
    // Default implementation for virtual function
    return (IsSelected() && !rkViewInfo.GameMode ? skSelectionTint : CColor::skWhite);
}

CColor CSceneNode::WireframeColor() const
{
    // Default implementation for virtual function
    return CColor::skWhite;
}

// ************ MAIN FUNCTIONALITY ************
void CSceneNode::OnLoadFinished()
{
    PostLoad();

    for (auto it = mChildren.begin(); it != mChildren.end(); it++)
        (*it)->OnLoadFinished();
}

void CSceneNode::Unparent()
{
    // May eventually want to reset XForm so global position = local position
    // Seems like a waste performance wise for the time being though
    if (mpParent)
        mpParent->RemoveChild(this);

    mpParent = nullptr;
}

void CSceneNode::RemoveChild(CSceneNode *pChild)
{
    for (auto it = mChildren.begin(); it != mChildren.end(); it++)
    {
        if (*it == pChild)
        {
            mChildren.erase(it);
            break;
        }
    }
}

void CSceneNode::DeleteChildren()
{
    for (auto it = mChildren.begin(); it != mChildren.end(); it++)
        delete *it;

    mChildren.clear();
}

void CSceneNode::SetInheritance(bool InheritPos, bool InheritRot, bool InheritScale)
{
    _mInheritsPosition = InheritPos;
    _mInheritsRotation = InheritRot;
    _mInheritsScale = InheritScale;
    MarkTransformChanged();
}

void CSceneNode::LoadModelMatrix()
{
    CGraphics::sMVPBlock.ModelMatrix = Transform().ToMatrix4f();
    CGraphics::UpdateMVPBlock();
}

void CSceneNode::BuildLightList(CGameArea *pArea)
{
    mLightCount = 0;
    mAmbientColor = CColor::skBlack;

    u32 Index = mLightLayerIndex;
    if ((pArea->NumLightLayers() <= Index) || (pArea->NumLights(Index) == 0)) Index = 0;

    struct SLightEntry {
        CLight *pLight;
        float Distance;

        SLightEntry(CLight *_pLight, float _Distance)
            : pLight(_pLight), Distance(_Distance) {}

        bool operator<(const SLightEntry& rkOther) {
            return (Distance < rkOther.Distance);
        }
    };
    std::vector<SLightEntry> LightEntries;

    // Default ambient color to white if there are no lights on the selected layer
    u32 NumLights = pArea->NumLights(Index);
    if (NumLights == 0) mAmbientColor = CColor::skWhite;

    for (u32 iLight = 0; iLight < NumLights; iLight++)
    {
        CLight* pLight = pArea->Light(Index, iLight);

        // Ambient lights should only be present one per layer; need to check how the game deals with multiple ambients
        if (pLight->Type() == eLocalAmbient)
            mAmbientColor = pLight->Color();

        // Other lights will be used depending which are closest to the node
        else
        {
            bool IsInRange = AABox().IntersectsSphere(pLight->Position(), pLight->GetRadius());

            if (IsInRange)
            {
                float Dist = mPosition.Distance(pLight->Position());
                LightEntries.push_back(SLightEntry(pLight, Dist));
            }
        }
    }

    // Determine which lights are closest
    std::sort(LightEntries.begin(), LightEntries.end());
    mLightCount = (LightEntries.size() > 8) ? 8 : LightEntries.size();

    for (u32 iLight = 0; iLight < mLightCount; iLight++)
        mLights[iLight] = LightEntries[iLight].pLight;
}

void CSceneNode::LoadLights(const SViewInfo& rkViewInfo)
{
    CGraphics::sNumLights = 0;
    CGraphics::ELightingMode Mode = (rkViewInfo.GameMode ? CGraphics::eWorldLighting : CGraphics::sLightMode);

    switch (Mode)
    {
    case CGraphics::eNoLighting:
        // No lighting: full white ambient, no dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = CColor::skWhite;
        break;

    case CGraphics::eBasicLighting:
        // Basic lighting: default ambient color, default dynamic lights
        CGraphics::SetDefaultLighting();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        break;

    case CGraphics::eWorldLighting:
        // World lighting: world ambient color, node dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = mAmbientColor;

        for (u32 iLight = 0; iLight < mLightCount; iLight++)
            mLights[iLight]->Load();
        break;
    }

    CGraphics::sPixelBlock.LightmapMultiplier = (Mode == CGraphics::eWorldLighting ? 1.f : 0.f);
    CGraphics::UpdateLightBlock();
}

void CSceneNode::DrawBoundingBox() const
{
    CDrawUtil::DrawWireCube(AABox(), CColor::skWhite);
}

void CSceneNode::DrawRotationArrow() const
{
    static TResPtr<CModel> spArrowModel = gResCache.GetResource("../resources/RotationArrow.cmdl");
    spArrowModel->Draw(eNoRenderOptions, 0);
}

void CSceneNode::AddSurfacesToRenderer(CRenderer *pRenderer, CModel *pModel, u32 MatSet, const SViewInfo& rkViewInfo)
{
    u32 SurfaceCount = pModel->GetSurfaceCount();

    for (u32 iSurf = 0; iSurf < SurfaceCount; iSurf++)
    {
        CAABox TransformedBox = pModel->GetSurfaceAABox(iSurf).Transformed(Transform());

        if (rkViewInfo.ViewFrustum.BoxInFrustum(TransformedBox))
        {
            if (!pModel->IsSurfaceTransparent(iSurf, MatSet))
                pRenderer->AddOpaqueMesh(this, (int) iSurf, TransformedBox, eDrawMesh);
            else
                pRenderer->AddTransparentMesh(this, (int) iSurf, TransformedBox, eDrawMesh);
        }
    }
}

// ************ TRANSFORM ************
void CSceneNode::Translate(const CVector3f& rkTranslation, ETransformSpace TransformSpace)
{
    switch (TransformSpace)
    {
    case eWorldTransform:
        mPosition += rkTranslation;
        break;
    case eLocalTransform:
        mPosition += mRotation * rkTranslation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Rotate(const CQuaternion& rkRotation, ETransformSpace TransformSpace)
{
    switch (TransformSpace)
    {
    case eWorldTransform:
        mRotation = rkRotation * mRotation;
        break;
    case eLocalTransform:
        mRotation *= rkRotation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Scale(const CVector3f& rkScale)
{
    // No support for stretch/skew world-space scaling; local only
    mScale *= rkScale;
    MarkTransformChanged();
}

void CSceneNode::ForceRecalculateTransform() const
{
    _mCachedTransform = CTransform4f::skIdentity;
    CalculateTransform(_mCachedTransform);
    _mCachedAABox = mLocalAABox.Transformed(_mCachedTransform);

    // Sync with children - only needed if caller hasn't marked transform changed already
    // If so, the children will already be marked
    if (!_mTransformDirty)
    {
        for (auto it = mChildren.begin(); it != mChildren.end(); it++)
            (*it)->MarkTransformChanged();
    }
    _mTransformDirty = false;
}

void CSceneNode::MarkTransformChanged() const
{
    if (!_mTransformDirty)
    {
        for (auto it = mChildren.begin(); it != mChildren.end(); it++)
            (*it)->MarkTransformChanged();
    }

    _mTransformDirty = true;
}

const CTransform4f& CSceneNode::Transform() const
{
    if (_mTransformDirty)
        ForceRecalculateTransform();

    return _mCachedTransform;
}

void CSceneNode::CalculateTransform(CTransform4f& rOut) const
{
    // Default implementation for virtual function
    rOut.Scale(AbsoluteScale());
    rOut.Rotate(AbsoluteRotation());
    rOut.Translate(AbsolutePosition());
}

// ************ GETTERS ************
CVector3f CSceneNode::AbsolutePosition() const
{
    CVector3f ret = mPosition;

    if ((mpParent) && (InheritsPosition()))
        ret += mpParent->AbsolutePosition();

    return ret;
}

CQuaternion CSceneNode::AbsoluteRotation() const
{
    CQuaternion ret = mRotation;

    if ((mpParent) && (InheritsRotation()))
        ret *= mpParent->AbsoluteRotation();

    return ret;
}

CVector3f CSceneNode::AbsoluteScale() const
{
    CVector3f ret = mScale;

    if ((mpParent) && (InheritsScale()))
        ret *= mpParent->AbsoluteScale();

    return ret;
}

CAABox CSceneNode::AABox() const
{
    if (_mTransformDirty)
        ForceRecalculateTransform();

    return _mCachedAABox;
}
