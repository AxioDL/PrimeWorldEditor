#include "CSkeleton.h"
#include "Core/Render/CBoneTransformData.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include <Math/MathUtil.h>

// ************ CBone ************
CBone::CBone(CSkeleton *pSkel)
    : mpSkeleton(pSkel)
{
}

void CBone::UpdateTransform(CBoneTransformData& rData, CAnimation *pAnim, float Time, bool AnchorRoot)
{
    CTransform4f& rTransform = rData[mID];
    rTransform.SetIdentity();

    if (pAnim)
        pAnim->EvaluateTransform(Time, mID, rTransform);

    if (!pAnim || !pAnim->HasTranslation(mID))
        rTransform.Translate(mPosition);

    if (mpParent)
        rTransform = rData[mpParent->ID()] * rTransform;

    if (AnchorRoot && IsRoot())
        rTransform.ZeroTranslation();

    for (u32 iChild = 0; iChild < mChildren.size(); iChild++)
        mChildren[iChild]->UpdateTransform(rData, pAnim, Time, AnchorRoot);

     rTransform *= mInvBind;
}

CVector3f CBone::TransformedPosition(const CBoneTransformData& rkData) const
{
    return rkData[mID] * AbsolutePosition();
}

bool CBone::IsRoot() const
{
    return (mpParent == nullptr);
}

// ************ CSkeleton ************
const float CSkeleton::skSphereRadius = 0.025f;

CSkeleton::CSkeleton()
    : mpRootBone(nullptr)
{
}

CSkeleton::~CSkeleton()
{
    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
        delete mBones[iBone];
}

CBone* CSkeleton::BoneByID(u32 BoneID) const
{
    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        if (mBones[iBone]->ID() == BoneID)
            return mBones[iBone];
    }

    return nullptr;
}

u32 CSkeleton::MaxBoneID() const
{
    u32 ID = 0;

    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        if (mBones[iBone]->ID() > ID)
            ID = mBones[iBone]->ID();
    }

    return ID;
}

void CSkeleton::UpdateTransform(CBoneTransformData& rData, CAnimation *pAnim, float Time, bool AnchorRoot)
{
    mpRootBone->UpdateTransform(rData, pAnim, Time, AnchorRoot);
}

void CSkeleton::Draw(FRenderOptions /*Options*/, const CBoneTransformData& rkData)
{
    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];
        CVector3f BonePos = pBone->TransformedPosition(rkData);

        // Draw bone
        CTransform4f Transform;
        Transform.Scale(skSphereRadius);
        Transform.Translate(BonePos);
        CGraphics::sMVPBlock.ModelMatrix = Transform;
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawSphere(CColor::skWhite);

        // Draw child links
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (u32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
        {
            CVector3f ChildPos = pBone->ChildByIndex(iChild)->TransformedPosition(rkData);
            CDrawUtil::DrawLine(BonePos, ChildPos);
        }
    }
}

std::pair<s32,float> CSkeleton::RayIntersect(const CRay& rkRay, const CBoneTransformData& rkData)
{
    std::pair<s32,float> Out(-1, FLT_MAX);

    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];
        CVector3f BonePos = rkData[pBone->ID()].ExtractTranslation();
        std::pair<bool,float> Intersect = Math::RaySphereIntersection(rkRay, BonePos, skSphereRadius);

        if (Intersect.first && Intersect.second < Out.second)
        {
            Out.first = pBone->ID();
            Out.second = Intersect.second;
        }
    }

    return Out;
}
