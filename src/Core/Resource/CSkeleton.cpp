#include "CSkeleton.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"

// ************ CBone ************
CBone::CBone(CSkeleton *pSkel)
    : mpSkeleton(pSkel)
{
}

void CBone::UpdateTransform(CAnimation *pAnim, float Time, bool AnchorRoot)
{
    mAnimTransform = CTransform4f::skIdentity;

    if (pAnim)
        pAnim->EvaluateTransform(Time, mID, mAnimTransform);

    if (!pAnim || !pAnim->HasTranslation(mID))
        mAnimTransform.Translate(mPosition);

    if (mpParent)
        mAnimTransform = mpParent->AnimTransform() * mAnimTransform;

    if (AnchorRoot && IsRoot())
        mAnimTransform.ZeroTranslation();

    mAbsPosDirty = true;

    for (u32 iChild = 0; iChild < mChildren.size(); iChild++)
        mChildren[iChild]->UpdateTransform(pAnim, Time, AnchorRoot);
}

bool CBone::IsRoot() const
{
    return (mpParent == nullptr);
}

CVector3f CBone::AbsolutePosition() const
{
    if (mAbsPosDirty)
    {
        mAbsolutePosition = (mAnimTransform * CVector3f::skZero);
        mAbsPosDirty = false;
    }

    return mAbsolutePosition;
}

// ************ CSkeleton ************
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

void CSkeleton::UpdateTransform(CAnimation *pAnim, float Time, bool AnchorRoot)
{
    mpRootBone->UpdateTransform(pAnim, Time, AnchorRoot);
}

void CSkeleton::Draw(FRenderOptions /*Options*/)
{
    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];

        // Draw bone
        CTransform4f Transform;
        Transform.Scale(0.025f);
        Transform.Translate(pBone->AbsolutePosition());
        CGraphics::sMVPBlock.ModelMatrix = Transform.ToMatrix4f();
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawSphere(CColor::skWhite);

        // Draw child links
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (u32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
            CDrawUtil::DrawLine(pBone->AbsolutePosition(), pBone->ChildByIndex(iChild)->AbsolutePosition());
    }
}
