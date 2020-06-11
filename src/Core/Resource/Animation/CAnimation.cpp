#include "CAnimation.h"
#include <Common/Math/CTransform4f.h>
#include <Common/Math/MathUtil.h>

#include <cfloat>

CAnimation::CAnimation(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
{
}

std::unique_ptr<CDependencyTree> CAnimation::BuildDependencyTree() const
{
    auto pTree = std::make_unique<CDependencyTree>();
    pTree->AddDependency(mpEventData);
    return pTree;
}

void CAnimation::EvaluateTransform(float Time, uint32 BoneID, CVector3f *pOutTranslation, CQuaternion *pOutRotation, CVector3f *pOutScale) const
{
    const bool kInterpolate = true;
    if (!pOutTranslation && !pOutRotation && !pOutScale) return;
    if (mDuration == 0.f) return;

    if (Time >= mDuration) Time = mDuration;
    if (Time >= FLT_EPSILON) Time -= FLT_EPSILON;
    float t = fmodf(Time, mTickInterval) / mTickInterval;
    uint32 LowKey = (uint32) (Time / mTickInterval);
    if (LowKey == (mNumKeys - 1)) LowKey = mNumKeys - 2;

    uint8 ScaleChannel = mBoneInfo[BoneID].ScaleChannelIdx;
    uint8 RotChannel = mBoneInfo[BoneID].RotationChannelIdx;
    uint8 TransChannel = mBoneInfo[BoneID].TranslationChannelIdx;

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

bool CAnimation::HasTranslation(uint32 BoneID) const
{
    return (mBoneInfo[BoneID].TranslationChannelIdx != 0xFF);
}
