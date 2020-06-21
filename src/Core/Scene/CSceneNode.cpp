#include "CSceneNode.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Resource/Area/CGameArea.h"
#include <Common/Macros.h>
#include <Common/Math/CTransform4f.h>

#include <algorithm>

uint32 CSceneNode::smNumNodes = 0;
CColor CSceneNode::skSelectionTint = CColor::Integral(39, 154, 167);

CSceneNode::CSceneNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent)
    : _mID(NodeID)
    , mpParent(pParent)
    , mpScene(pScene)
{
    smNumNodes++;

    if (mpParent)
        mpParent->mChildren.push_back(this);
}

CSceneNode::~CSceneNode()
{
    smNumNodes--;
    DeleteChildren();
}

// ************ VIRTUAL ************
void CSceneNode::DrawSelection()
{
    // Default implementation for virtual function
    CDrawUtil::DrawWireCube(AABox(), CColor::White());
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
    return (IsSelected() && !rkViewInfo.GameMode ? skSelectionTint : CColor::White());
}

CColor CSceneNode::WireframeColor() const
{
    // Default implementation for virtual function
    return CColor::White();
}

// ************ MAIN FUNCTIONALITY ************
void CSceneNode::OnLoadFinished()
{
    PostLoad();

    for (auto* child : mChildren)
        child->OnLoadFinished();
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
    for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
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
    for (auto* child : mChildren)
        delete child;

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
    CGraphics::sMVPBlock.ModelMatrix = Transform();
    CGraphics::UpdateMVPBlock();
}

void CSceneNode::BuildLightList(CGameArea *pArea)
{
    mLightCount = 0;
    mAmbientColor = CColor::TransparentBlack();

    size_t Index = mLightLayerIndex;
    if (pArea->NumLightLayers() <= Index || pArea->NumLights(Index) == 0)
        Index = 0;

    struct SLightEntry {
        CLight *pLight;
        float Distance;

        SLightEntry(CLight *_pLight, float _Distance)
            : pLight(_pLight), Distance(_Distance) {}

        bool operator<(const SLightEntry& rkOther) const {
            return (Distance < rkOther.Distance);
        }
    };
    std::vector<SLightEntry> LightEntries;

    // Default ambient color to white if there are no lights on the selected layer
    const size_t NumLights = pArea->NumLights(Index);
    if (NumLights == 0)
        mAmbientColor = CColor::TransparentWhite();

    for (size_t iLight = 0; iLight < NumLights; iLight++)
    {
        CLight* pLight = pArea->Light(Index, iLight);

        // Ambient lights should only be present one per layer; need to check how the game deals with multiple ambients
        if (pLight->Type() == ELightType::LocalAmbient)
        {
            mAmbientColor = pLight->Color();
        }
        else // Other lights will be used depending which are closest to the node
        {
            const bool IsInRange = AABox().IntersectsSphere(pLight->Position(), pLight->GetRadius());

            if (IsInRange)
            {
                const float Dist = mPosition.Distance(pLight->Position());
                LightEntries.push_back(SLightEntry(pLight, Dist));
            }
        }
    }

    // Determine which lights are closest
    std::sort(LightEntries.begin(), LightEntries.end());
    mLightCount = (LightEntries.size() > 8) ? 8 : LightEntries.size();

    for (uint32 iLight = 0; iLight < mLightCount; iLight++)
        mLights[iLight] = LightEntries[iLight].pLight;
}

void CSceneNode::LoadLights(const SViewInfo& rkViewInfo)
{
    CGraphics::sNumLights = 0;
    CGraphics::ELightingMode Mode = (rkViewInfo.GameMode ? CGraphics::ELightingMode::World : CGraphics::sLightMode);

    switch (Mode)
    {
    case CGraphics::ELightingMode::None:
        // No lighting: full white ambient, no dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = CColor::TransparentWhite();
        break;

    case CGraphics::ELightingMode::Basic:
        // Basic lighting: default ambient color, default dynamic lights
        CGraphics::SetDefaultLighting();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        break;

    case CGraphics::ELightingMode::World:
        // World lighting: world ambient color, node dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = mAmbientColor;

        for (uint32 iLight = 0; iLight < mLightCount; iLight++)
            mLights[iLight]->Load();
        break;
    }

    CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();

    CGraphics::sPixelBlock.LightmapMultiplier = (Mode == CGraphics::ELightingMode::World ? 1.f : 0.f);
    CGraphics::UpdateLightBlock();
}

void CSceneNode::AddModelToRenderer(CRenderer *pRenderer, CModel *pModel, size_t MatSet)
{
    ASSERT(pModel);

    if (!pModel->HasTransparency(MatSet))
    {
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawMesh);
    }
    else
    {
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawOpaqueParts);
        pRenderer->AddMesh(this, -1, AABox(), true, ERenderCommand::DrawTransparentParts);
    }
}

void CSceneNode::DrawModelParts(CModel *pModel, FRenderOptions Options, size_t MatSet, ERenderCommand RenderCommand)
{
    // Common rendering functionality
    if (RenderCommand == ERenderCommand::DrawMesh)
    {
        pModel->Draw(Options, MatSet);
    }
    else
    {
        const bool DrawOpaque = RenderCommand == ERenderCommand::DrawOpaqueParts;
        const bool DrawTransparent = RenderCommand == ERenderCommand::DrawTransparentParts;

        for (size_t iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
        {
            const bool ShouldRender = (DrawOpaque && DrawTransparent) ||
                                      (DrawOpaque && !pModel->IsSurfaceTransparent(iSurf, MatSet)) ||
                                      (DrawTransparent && pModel->IsSurfaceTransparent(iSurf, MatSet));

            if (ShouldRender)
                pModel->DrawSurface(Options, iSurf, MatSet);
        }
    }
}

void CSceneNode::DrawBoundingBox() const
{
    CDrawUtil::DrawWireCube(AABox(), CColor::White());
}

void CSceneNode::DrawRotationArrow() const
{
    static TResPtr<CModel> spArrowModel = gpEditorStore->LoadResource("RotationArrow.CMDL");
    spArrowModel->Draw(ERenderOption::None, 0);
}

// ************ TRANSFORM ************
void CSceneNode::Translate(const CVector3f& rkTranslation, ETransformSpace TransformSpace)
{
    switch (TransformSpace)
    {
    case ETransformSpace::World:
        mPosition += rkTranslation;
        break;
    case ETransformSpace::Local:
        mPosition += mRotation * rkTranslation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Rotate(const CQuaternion& rkRotation, ETransformSpace TransformSpace)
{
    switch (TransformSpace)
    {
    case ETransformSpace::World:
        mRotation = rkRotation * mRotation;
        break;
    case ETransformSpace::Local:
        mRotation *= rkRotation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Rotate(const CQuaternion& rkRotation, const CVector3f& rkPivot, const CQuaternion& rkPivotRotation, ETransformSpace TransformSpace)
{
    switch (TransformSpace)
    {
    case ETransformSpace::World:
        mPosition = rkPivot + (rkRotation * (mPosition - rkPivot));
        break;
    case ETransformSpace::Local:
        mPosition = rkPivot + ((rkPivotRotation * rkRotation * rkPivotRotation.Inverse()) * (mPosition - rkPivot));
        break;
    }
    Rotate(rkRotation, TransformSpace);
}

void CSceneNode::Scale(const CVector3f& rkScale)
{
    mScale *= rkScale;
    MarkTransformChanged();
}

void CSceneNode::Scale(const CVector3f& rkScale, const CVector3f& rkPivot)
{
    mPosition = rkPivot + ((mPosition - rkPivot) * rkScale * rkScale);
    Scale(rkScale);
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
        for (auto* child : mChildren)
            child->MarkTransformChanged();
    }
    _mTransformDirty = false;
}

void CSceneNode::MarkTransformChanged() const
{
    if (!_mTransformDirty)
    {
        for (auto* child : mChildren)
            child->MarkTransformChanged();
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

    if (mpParent != nullptr && InheritsPosition())
        ret += mpParent->AbsolutePosition();

    return ret;
}

CQuaternion CSceneNode::AbsoluteRotation() const
{
    CQuaternion ret = mRotation;

    if (mpParent != nullptr && InheritsRotation())
        ret *= mpParent->AbsoluteRotation();

    return ret;
}

CVector3f CSceneNode::AbsoluteScale() const
{
    CVector3f ret = mScale;

    if (mpParent != nullptr && InheritsScale())
        ret *= mpParent->AbsoluteScale();

    return ret;
}

CAABox CSceneNode::AABox() const
{
    if (_mTransformDirty)
        ForceRecalculateTransform();

    return _mCachedAABox;
}
