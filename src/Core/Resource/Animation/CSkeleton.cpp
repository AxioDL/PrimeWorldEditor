#include "CSkeleton.h"
#include "Core/Render/CBoneTransformData.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include <Common/Macros.h>
#include <Common/Math/MathUtil.h>

#include <algorithm>
#include <cfloat>

// ************ CBone ************
CBone::CBone(CSkeleton *pSkel)
    : mpSkeleton(pSkel)
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
        TransformInfo.Position = CVector3f::Zero();

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
    for (auto* child : mChildren)
        child->UpdateTransform(rData, TransformInfo, pAnim, Time, AnchorRoot);
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
    return mpParent == nullptr || mpParent->Parent() == nullptr;
}

// ************ CSkeleton ************

CSkeleton::CSkeleton(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CSkeleton::~CSkeleton() = default;

CBone* CSkeleton::BoneByID(uint32 BoneID) const
{
    const auto iter = std::find_if(mBones.begin(), mBones.end(),
                                   [BoneID](const auto& bone) { return bone->ID() == BoneID; });

    if (iter == mBones.cend())
        return nullptr;

    return iter->get();
}

CBone* CSkeleton::BoneByName(std::string_view name) const
{
    const auto iter = std::find_if(mBones.begin(), mBones.end(),
                                   [&name](const auto& bone) { return bone->Name() == name; });

    if (iter == mBones.cend())
        return nullptr;

    return iter->get();
}

uint32 CSkeleton::MaxBoneID() const
{
    const auto iter = std::max_element(mBones.cbegin(), mBones.cend(),
                                       [](const auto& a, const auto& b) { return a->ID() < b->ID(); });

    if (iter == mBones.cend())
    {
        return 0;
    }

    return (*iter)->ID();
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
    for (const auto& pBone : mBones)
    {
        const CVector3f BonePos = pkData ? pBone->TransformedPosition(*pkData) : pBone->Position();

        // Draw the bone's local XYZ axes for selected bones
        if (pBone->IsSelected())
        {
            const CQuaternion BoneRot = pkData ? pBone->TransformedRotation(*pkData) : pBone->Rotation();
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.XAxis(), CColor::Red());
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.YAxis(), CColor::Green());
            CDrawUtil::DrawLine(BonePos, BonePos + BoneRot.ZAxis(), CColor::Blue());
        }

        // Draw child links
        for (size_t iChild = 0; iChild < pBone->NumChildren(); iChild++)
        {
            const CBone *pChild = pBone->ChildByIndex(iChild);
            const CVector3f ChildPos = pkData ? pChild->TransformedPosition(*pkData) : pChild->Position();
            CDrawUtil::DrawLine(BonePos, ChildPos);
        }
    }

    // Draw bone spheres
    const CTransform4f BaseTransform = CGraphics::sMVPBlock.ModelMatrix;

    for (const auto& pBone : mBones)
    {
        const CVector3f BonePos = pkData ? pBone->TransformedPosition(*pkData) : pBone->Position();

        CTransform4f Transform;
        Transform.Scale(skSphereRadius);
        Transform.Translate(BonePos);
        CGraphics::sMVPBlock.ModelMatrix = Transform * BaseTransform;
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawSphere(pBone->IsSelected() ? CColor::Red() : CColor::White());
    }
}

std::pair<int32,float> CSkeleton::RayIntersect(const CRay& rkRay, const CBoneTransformData& rkData) const
{
    std::pair<int32,float> Out(-1, FLT_MAX);

    for (const auto& pBone : mBones)
    {
        const CVector3f BonePos = pBone->TransformedPosition(rkData);
        const std::pair<bool, float> Intersect = Math::RaySphereIntersection(rkRay, BonePos, skSphereRadius);

        if (Intersect.first && Intersect.second < Out.second)
        {
            Out.first = pBone->ID();
            Out.second = Intersect.second;
        }
    }

    return Out;
}
