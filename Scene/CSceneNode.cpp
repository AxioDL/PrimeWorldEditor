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

CSceneNode::CSceneNode(CSceneManager *pScene, CSceneNode *pParent)
{
    smNumNodes++;
    mpScene = pScene;
    mpParent = pParent;

    mPosition = CVector3f::skZero;
    mRotation = CQuaternion::skIdentity;
    mScale = CVector3f::skOne;
    _mTransformOutdated = true;

    _mInheritsPosition = true;
    _mInheritsRotation = true;
    _mInheritsScale = true;

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
std::string CSceneNode::PrefixedName() const
{
    return Name();
}

void CSceneNode::DrawSelection()
{
    // Default implementation for virtual function
    CDrawUtil::DrawWireCube(AABox(), CColor::skWhite);
}

void CSceneNode::RayAABoxIntersectTest(CRayCollisionTester& Tester)
{
    // Default implementation for virtual function
    std::pair<bool,float> result = AABox().IntersectsRay(Tester.Ray());

    if (result.first)
        Tester.AddNode(this, -1, result.second);
}

bool CSceneNode::IsVisible() const
{
    return mVisible;
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

    u32 LayerCount = pArea->GetLightLayerCount();

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

    for (u32 iLayer = 0; iLayer < LayerCount; iLayer++)
    {
        u32 LightCount = pArea->GetLightCount(iLayer);

        for (u32 iLight = 0; iLight < LightCount; iLight++)
        {
            CLight* pLight = pArea->GetLight(iLayer, iLight);

            // Directional/Spot lights take priority over custom lights.
            if ((pLight->GetType() == eDirectional) || (pLight->GetType() == eSpot))
                LightEntries.push_back(SLightEntry(pLight, 0));

            // Custom lights will be used depending which are closest to the node
            else if (pLight->GetType() == eCustom)
            {
                bool IsInRange = AABox().IntersectsSphere(pLight->GetPosition(), pLight->GetRadius());

                if (IsInRange)
                {
                    float Dist = mPosition.Distance(pLight->GetPosition());
                    LightEntries.push_back(SLightEntry(pLight, Dist));
                }
            }
        }
    }

    // Determine which lights are closest
    std::sort(LightEntries.begin(), LightEntries.end());
    mLightCount = (LightEntries.size() > 8) ? 8 : LightEntries.size();

    for (u32 i = 0; i < mLightCount; i++)
        mLights[i] = LightEntries[i].pLight;
}

void CSceneNode::LoadLights()
{
    CGraphics::sNumLights = 0;

    if (CGraphics::sLightMode == CGraphics::BasicLighting)
        CGraphics::SetDefaultLighting();

    else if (CGraphics::sLightMode == CGraphics::WorldLighting)
        for (u32 iLight = 0; iLight < mLightCount; iLight++)
            mLights[iLight]->Load();

    CGraphics::UpdateLightBlock();
}

void CSceneNode::DrawBoundingBox()
{
    CDrawUtil::DrawWireCube(AABox(), CColor::skWhite);
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

void CSceneNode::Scale(const CVector3f& scale, ETransformSpace transformSpace)
{
    mScale *= scale;
    MarkTransformChanged();
}

void CSceneNode::UpdateTransform()
{
    if (_mTransformOutdated)
    {
        ForceRecalculateTransform();
        _mTransformOutdated = false;
    }
}

void CSceneNode::ForceRecalculateTransform()
{
    _mCachedTransform = CTransform4f::skIdentity;
    _mCachedTransform.Scale(AbsoluteScale());
    _mCachedTransform.Rotate(AbsoluteRotation());
    _mCachedTransform.Translate(AbsolutePosition());
    _mCachedAABox = mLocalAABox.Transformed(_mCachedTransform);

    // Sync with children - only needed if caller hasn't marked transform changed already
    // If so, the children will already be marked
    if (!_mTransformOutdated)
    {
        for (auto it = mChildren.begin(); it != mChildren.end(); it++)
            (*it)->MarkTransformChanged();
    }
    _mTransformOutdated = false;
}

void CSceneNode::MarkTransformChanged()
{
    if (!_mTransformOutdated)
    {
        for (auto it = mChildren.begin(); it != mChildren.end(); it++)
            (*it)->MarkTransformChanged();
    }

    _mTransformOutdated = true;
}

const CTransform4f& CSceneNode::Transform()
{
    if (_mTransformOutdated)
        ForceRecalculateTransform();

    return _mCachedTransform;
}

// ************ GETTERS ************
std::string CSceneNode::Name() const
{
    return mName;
}

CSceneNode* CSceneNode::Parent() const
{
    return mpParent;
}

CSceneManager* CSceneNode::Scene()
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

CAABox CSceneNode::AABox()
{
    if (_mTransformOutdated)
        ForceRecalculateTransform();

    return _mCachedAABox;
}

CVector3f CSceneNode::CenterPoint()
{
    return AABox().Center();
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
void CSceneNode::SetName(const std::string& Name)
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
