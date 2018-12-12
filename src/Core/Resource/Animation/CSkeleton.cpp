#include "CSkeleton.h"
#include "Core/Render/CBoneTransformData.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include <Common/Macros.h>
#include <Common/Math/MathUtil.h>

// ************ CBone ************
CBone::CBone(CSkeleton *pSkel)
    : mpSkeleton(pSkel)
    , mSelected(false)
{
}

void CBone::UpdateTransform(CBoneTransformData& rData, const SBoneTransformInfo& rkParentTransform, CAnimation *pAnim, float Time, bool AnchorRoot)
{
    // Get transform data
    SBoneTransformInfo TransformInfo;
    TransformInfo.Position = mLocalPosition;

    if (pAnim)
        pAnim->EvaluateTransform(Time, mID, &TransformInfo.Position, &TransformInfo.Rotation, &TransformInfo.Scale);

    if (AnchorRoot && IsRoot())
        TransformInfo.Position = CVector3f::skZero;

    // Apply parent transform
    TransformInfo.Position = rkParentTransform.Position + (rkParentTransform.Rotation * (rkParentTransform.Scale * TransformInfo.Position));
    TransformInfo.Rotation = rkParentTransform.Rotation * TransformInfo.Rotation;

    // Calculate transform
    CTransform4f& rTransform = rData[mID];
    rTransform.SetIdentity();
    rTransform.Scale(TransformInfo.Scale);
    rTransform.Rotate(TransformInfo.Rotation);
    rTransform.Translate(TransformInfo.Position);
    rTransform *= mInvBind;

    // Calculate children
    for (uint32 iChild = 0; iChild < mChildren.size(); iChild++)
        mChildren[iChild]->UpdateTransform(rData, TransformInfo, pAnim, Time, AnchorRoot);
}

CVector3f CBone::TransformedPosition(const CBoneTransformData& rkData) const
{
    return rkData[mID] * Position();
}

CQuaternion CBone::TransformedRotation(const CBoneTransformData &rkData) const
{
    return rkData[mID] * Rotation();
}

bool CBone::IsRoot() const
{
    // In Retro's engine most skeletons have another bone named Skeleton_Root parented directly under the
    // actual root bone... that bone sometimes acts as the actual root (transforming the entire skeleton),
    // so we need to account for both
    return (mpParent == nullptr || mpParent->Parent() == nullptr);
}

// ************ CSkeleton ************
const float CSkeleton::skSphereRadius = 0.025f;

CSkeleton::CSkeleton(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mpRootBone(nullptr)
{
}

CSkeleton::~CSkeleton()
{
    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
        delete mBones[iBone];
}

CBone* CSkeleton::BoneByID(uint32 BoneID) const
{
    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        if (mBones[iBone]->ID() == BoneID)
            return mBones[iBone];
    }

    return nullptr;
}

CBone* CSkeleton::BoneByName(const TString& rkBoneName) const
{
    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        if (mBones[iBone]->Name() == rkBoneName)
            return mBones[iBone];
    }

    return nullptr;
}

uint32 CSkeleton::MaxBoneID() const
{
    uint32 ID = 0;

    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        if (mBones[iBone]->ID() > ID)
            ID = mBones[iBone]->ID();
    }

    return ID;
}

void CSkeleton::UpdateTransform(CBoneTransformData& rData, CAnimation *pAnim, float Time, bool AnchorRoot)
{
    ASSERT(rData.NumTrackedBones() >= MaxBoneID());
    mpRootBone->UpdateTransform(rData, SBoneTransformInfo(), pAnim, Time, AnchorRoot);
}

void CSkeleton::Draw(FRenderOptions /*Options*/, const CBoneTransformData *pkData)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glLineWidth(1.f);

    // Draw all child links first to minimize model matrix swaps.
    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];
        CVector3f BonePos = pkData ? pBone->TransformedPosition(*pkData) : pBone->Position();

        // Draw the bone's local XYZ axes for selected bones
        if (pBone->IsSelected())
        {
            CQuaternion BoneRot = pkData ? pBone->TransformedRotation(*pkData) : pBone->Rotation();
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.XAxis(), CColor::skRed);
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.YAxis(), CColor::skGreen);
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.ZAxis(), CColor::skBlue);
        }

        // Draw child links
        for (uint32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
        {
            CBone *pChild = pBone->ChildByIndex(iChild);
            CVector3f ChildPos = pkData ? pChild->TransformedPosition(*pkData) : pChild->Position();
            CDrawUtil::DrawLine(BonePos, ChildPos);
        }
    }

    // Draw bone spheres
    CTransform4f BaseTransform = CGraphics::sMVPBlock.ModelMatrix;

    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];
        CVector3f BonePos = pkData ? pBone->TransformedPosition(*pkData) : pBone->Position();

        CTransform4f Transform;
        Transform.Scale(skSphereRadius);
        Transform.Translate(BonePos);
        CGraphics::sMVPBlock.ModelMatrix = Transform * BaseTransform;
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawSphere(pBone->IsSelected() ? CColor::skRed : CColor::skWhite);
    }
}

std::pair<int32,float> CSkeleton::RayIntersect(const CRay& rkRay, const CBoneTransformData& rkData)
{
    std::pair<int32,float> Out(-1, FLT_MAX);

    for (uint32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];
        CVector3f BonePos = pBone->TransformedPosition(rkData);
        std::pair<bool,float> Intersect = Math::RaySphereIntersection(rkRay, BonePos, skSphereRadius);

        if (Intersect.first && Intersect.second < Out.second)
        {
            Out.first = pBone->ID();
            Out.second = Intersect.second;
        }
    }

    return Out;
}
