#include "CAnimation.h"
#include <Math/CTransform4f.h>
#include <Math/MathUtil.h>

CAnimation::CAnimation(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mDuration(0.f)
    , mTickInterval(0.0333333f)
    , mNumKeys(0)
{
    for (u32 iBone = 0; iBone < 100; iBone++)
    {
        mBoneInfo[iBone].TranslationChannelIdx = 0xFF;
        mBoneInfo[iBone].RotationChannelIdx = 0xFF;
        mBoneInfo[iBone].ScaleChannelIdx = 0xFF;
    }
}

CDependencyTree* CAnimation::BuildDependencyTree() const
{
    CDependencyTree *pTree = new CDependencyTree(ID());
    pTree->AddDependency(mEventData);
    return pTree;
}

void CAnimation::EvaluateTransform(float Time, u32 BoneID, CVector3f *pOutTranslation, CQuaternion *pOutRotation, CVector3f *pOutScale) const
{
    const bool kInterpolate = true;
    if (!pOutTranslation && !pOutRotation && !pOutScale) return;
    if (mDuration == 0.f) return;

    if (Time >= mDuration) Time = mDuration;
    if (Time >= FLT_EPSILON) Time -= FLT_EPSILON;
    float t = fmodf(Time, mTickInterval) / mTickInterval;
    u32 LowKey = (u32) (Time / mTickInterval);
    if (LowKey == (mNumKeys - 1)) LowKey = mNumKeys - 2;

    u8 ScaleChannel = mBoneInfo[BoneID].ScaleChannelIdx;
    u8 RotChannel = mBoneInfo[BoneID].RotationChannelIdx;
    u8 TransChannel = mBoneInfo[BoneID].TranslationChannelIdx;

    if (ScaleChannel != 0xFF && pOutScale)
    {
        const CVector3f& rkLow = mScaleChannels[ScaleChannel][LowKey];
        const CVector3f& rkHigh = mScaleChannels[ScaleChannel][LowKey + 1];
        *pOutScale = (kInterpolate ? Math::Lerp<CVector3f>(rkLow, rkHigh, t) : rkLow);
    }

    if (RotChannel != 0xFF && pOutRotation)
    {
        const CQuaternion& rkLow = mRotationChannels[RotChannel][LowKey];
        const CQuaternion& rkHigh = mRotationChannels[RotChannel][LowKey + 1];
        *pOutRotation = (kInterpolate ? rkLow.Slerp(rkHigh, t) : rkLow);
    }

    if (TransChannel != 0xFF && pOutTranslation)
    {
        const CVector3f& rkLow = mTranslationChannels[TransChannel][LowKey];
        const CVector3f& rkHigh = mTranslationChannels[TransChannel][LowKey + 1];
        *pOutTranslation = (kInterpolate ? Math::Lerp<CVector3f>(rkLow, rkHigh, t) : rkLow);
    }
}

bool CAnimation::HasTranslation(u32 BoneID) const
{
    return (mBoneInfo[BoneID].TranslationChannelIdx != 0xFF);
}
