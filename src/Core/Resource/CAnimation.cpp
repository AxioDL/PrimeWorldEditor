#include "CAnimation.h"
#include <Math/CTransform4f.h>
#include <Math/MathUtil.h>

CAnimation::CAnimation()
    : mDuration(0.f)
    , mTickInterval(0.0333333f)
    , mNumKeys(0)
{
    for (u32 iBone = 0; iBone < 100; iBone++)
    {
        mBoneInfo[iBone].RotationChannelIdx = 0xFF;
        mBoneInfo[iBone].TranslationChannelIdx = 0xFF;
    }
}

void CAnimation::EvaluateTransform(float Time, u32 BoneID, CTransform4f& rOut) const
{
    if (mDuration == 0.f) return;

    if (Time >= mDuration) Time = mDuration;
    if (Time >= FLT_EPSILON) Time -= FLT_EPSILON;
    float t = fmodf(Time, mTickInterval) / mTickInterval;
    u32 LowKey = (u32) (Time / mTickInterval);
    if (LowKey == (mNumKeys - 1)) LowKey = mNumKeys - 2;

    u8 RotChannel = mBoneInfo[BoneID].RotationChannelIdx;
    u8 TransChannel = mBoneInfo[BoneID].TranslationChannelIdx;

    if (RotChannel != 0xFF)
    {
        const CQuaternion& rkLow = mRotationChannels[RotChannel][LowKey];
        const CQuaternion& rkHigh = mRotationChannels[RotChannel][LowKey + 1];
        rOut.Rotate( rkLow.Slerp(rkHigh, t) );
    }

    if (TransChannel != 0xFF)
    {
        const CVector3f& rkLow = mTranslationChannels[TransChannel][LowKey];
        const CVector3f& rkHigh = mTranslationChannels[TransChannel][LowKey + 1];
        rOut.Translate( Math::Lerp<CVector3f>(rkLow, rkHigh, t) );
    }
}

bool CAnimation::HasTranslation(u32 BoneID) const
{
    return (mBoneInfo[BoneID].TranslationChannelIdx != 0xFF);
}
