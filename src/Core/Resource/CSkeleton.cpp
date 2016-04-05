#include "CSkeleton.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"

// ************ CBone ************
CBone::CBone(CSkeleton *pSkel)
    : mpSkeleton(pSkel)
{
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

void CSkeleton::Draw(FRenderOptions /*Options*/)
{
    for (u32 iBone = 0; iBone < mBones.size(); iBone++)
    {
        CBone *pBone = mBones[iBone];

        // Draw bone
        CTransform4f Transform;
        Transform.Scale(0.01f);
        Transform.Translate(pBone->Position());
        CGraphics::sMVPBlock.ModelMatrix = Transform.ToMatrix4f();
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawSphere(CColor::skWhite);

        // Draw child links
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (u32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
            CDrawUtil::DrawLine(pBone->Position(), pBone->ChildByIndex(iChild)->Position());
    }
}
