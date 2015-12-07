#include "CSceneNode.h"
#include <Core/CRenderer.h>
#include <gtc/quaternion.hpp>
#include <gtx/transform.hpp>
#include <Common/AnimUtil.h>
#include <Common/CTransform4f.h>
#include <Resource/CGameArea.h>
#include <Core/CGraphics.h>
#include <Core/CDrawUtil.h>
#include <algorithm>

u32 CSceneNode::smNumNodes = 0;
CColor CSceneNode::skSelectionTint((u8) 39, 154, 167, 255);

CSceneNode::CSceneNode(CSceneManager *pScene, CSceneNode *pParent)
{
    smNumNodes++;
    mpScene = pScene;
    mpParent = pParent;

    mPosition = CVector3f::skZero;
    mRotation = CQuaternion::skIdentity;
    mScale = CVector3f::skOne;
    _mTransformDirty = true;

    _mInheritsPosition = true;
    _mInheritsRotation = true;
    _mInheritsScale = true;

    mLightLayerIndex = 0;
    mLightCount = 0;

    mMouseHovering = false;
    mSelected = false;
    mVisible = true;

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

void CSceneNode::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& /*ViewInfo*/)
{
    // Default implementation for virtual function
    std::pair<bool,float> result = AABox().IntersectsRay(Tester.Ray());

    if (result.first)
        Tester.AddNode(this, -1, result.second);
}

bool CSceneNode::IsVisible() const
{
    // Default implementation for virtual function
    return mVisible;
}

CColor CSceneNode::TintColor(const SViewInfo& ViewInfo) const
{
    // Default implementation for virtual function
    return (IsSelected() && !ViewInfo.GameMode ? skSelectionTint : CColor::skWhite);
}

CColor CSceneNode::WireframeColor() const
{
    // Default implementation for virtual function
    return CColor::skWhite;
}

// ************ MAIN FUNCTIONALITY ************
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

    u32 index = mLightLayerIndex;
    if ((pArea->GetLightLayerCount() <= index) || (pArea->GetLightCount(index) == 0)) index = 0;

    struct SLightEntry {
        CLight *pLight;
        float Distance;

        SLightEntry(CLight *_pLight, float _Distance)
            : pLight(_pLight), Distance(_Distance) {}

        bool operator<(const SLightEntry& other) {
            return (Distance < other.Distance);
        }
    };
    std::vector<SLightEntry> LightEntries;

    // Default ambient color to white if there are no lights on the selected layer
    u32 numLights = pArea->GetLightCount(index);
    if (numLights == 0) mAmbientColor = CColor::skWhite;

    for (u32 iLight = 0; iLight < numLights; iLight++)
    {
        CLight* pLight = pArea->GetLight(index, iLight);

        // Ambient lights should only be present one per layer; need to check how the game deals with multiple ambients
        if (pLight->GetType() == eLocalAmbient)
            mAmbientColor = pLight->GetColor();

        // Other lights will be used depending which are closest to the node
        else
        {
            bool IsInRange = AABox().IntersectsSphere(pLight->GetPosition(), pLight->GetRadius());

            if (IsInRange)
            {
                float Dist = mPosition.Distance(pLight->GetPosition());
                LightEntries.push_back(SLightEntry(pLight, Dist));
            }
        }
    }

    // Determine which lights are closest
    std::sort(LightEntries.begin(), LightEntries.end());
    mLightCount = (LightEntries.size() > 8) ? 8 : LightEntries.size();

    for (u32 i = 0; i < mLightCount; i++)
        mLights[i] = LightEntries[i].pLight;
}

void CSceneNode::LoadLights(const SViewInfo& ViewInfo)
{
    CGraphics::sNumLights = 0;

    if (CGraphics::sLightMode == CGraphics::eWorldLighting || ViewInfo.GameMode)
    {
        // World lighting: world ambient color, node dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = mAmbientColor.ToVector4f();

        for (u32 iLight = 0; iLight < mLightCount; iLight++)
            mLights[iLight]->Load();
    }

    else if (CGraphics::sLightMode == CGraphics::eBasicLighting)
    {
        // Basic lighting: default ambient color, default dynamic lights
        CGraphics::SetDefaultLighting();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();
    }

    else if (CGraphics::sLightMode == CGraphics::eNoLighting)
    {
        // No lighting: default ambient color, no dynamic lights
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();
    }

    CGraphics::UpdateLightBlock();
}

void CSceneNode::DrawBoundingBox() const
{
    CDrawUtil::DrawWireCube(AABox(), CColor::skWhite);
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
void CSceneNode::Translate(const CVector3f& translation, ETransformSpace transformSpace)
{
    switch (transformSpace)
    {
    case eWorldTransform:
        mPosition += translation;
        break;
    case eLocalTransform:
        mPosition += mRotation * translation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Rotate(const CQuaternion& rotation, ETransformSpace transformSpace)
{
    switch (transformSpace)
    {
    case eWorldTransform:
        mRotation = rotation * mRotation;
        break;
    case eLocalTransform:
        mRotation *= rotation;
        break;
    }
    MarkTransformChanged();
}

void CSceneNode::Scale(const CVector3f& scale)
{
    // No support for stretch/skew world-space scaling; local only
    mScale *= scale;
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
TString CSceneNode::Name() const
{
    return mName;
}

CSceneNode* CSceneNode::Parent() const
{
    return mpParent;
}

CSceneManager* CSceneNode::Scene() const
{
    return mpScene;
}

CVector3f CSceneNode::LocalPosition() const
{
    return mPosition;
}

CVector3f CSceneNode::AbsolutePosition() const
{
    CVector3f ret = mPosition;

    if ((mpParent) && (InheritsPosition()))
        ret += mpParent->AbsolutePosition();

    return ret;
}

CQuaternion CSceneNode::LocalRotation() const
{
    return mRotation;
}

CQuaternion CSceneNode::AbsoluteRotation() const
{
    CQuaternion ret = mRotation;

    if ((mpParent) && (InheritsRotation()))
        ret *= mpParent->AbsoluteRotation();

    return ret;
}

CVector3f CSceneNode::LocalScale() const
{
    return mScale;
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

CVector3f CSceneNode::CenterPoint() const
{
    return AABox().Center();
}

u32 CSceneNode::LightLayerIndex() const
{
    return mLightLayerIndex;
}

bool CSceneNode::MarkedVisible() const
{
    // The reason I have this function is because the instance view needs to know whether a node is marked
    // visible independently from other factors that may affect node visibility (as returned by IsVisible()).
    // It's a little confusing, so maybe there's a better way to set this up.
    return mVisible;
}

bool CSceneNode::IsMouseHovering() const
{
    return mMouseHovering;
}

bool CSceneNode::IsSelected() const
{
    return mSelected;
}

bool CSceneNode::InheritsPosition() const
{
    return _mInheritsPosition;
}

bool CSceneNode::InheritsRotation() const
{
    return _mInheritsRotation;
}

bool CSceneNode::InheritsScale() const
{
    return _mInheritsScale;
}

// ************ SETTERS ************
void CSceneNode::SetName(const TString& Name)
{
    mName = Name;
}

void CSceneNode::SetPosition(const CVector3f& position)
{
    mPosition = position;
    MarkTransformChanged();
}

void CSceneNode::SetRotation(const CQuaternion& rotation)
{
    mRotation = rotation;
    MarkTransformChanged();
}

void CSceneNode::SetRotation(const CVector3f& rotEuler)
{
    mRotation = CQuaternion::FromEuler(rotEuler);
    MarkTransformChanged();
}

void CSceneNode::SetScale(const CVector3f& scale)
{
    mScale = scale;
    MarkTransformChanged();
}

void CSceneNode::SetLightLayerIndex(u32 index)
{
    mLightLayerIndex = index;
}

void CSceneNode::SetMouseHovering(bool Hovering)
{
    mMouseHovering = Hovering;
}

void CSceneNode::SetSelected(bool Selected)
{
    mSelected = Selected;
}

void CSceneNode::SetVisible(bool Visible)
{
    mVisible = Visible;
}
