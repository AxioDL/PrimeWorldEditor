#ifndef CANIMATION_H
#define CANIMATION_H

#include "CResource.h"
#include <Math/CQuaternion.h>
#include <Math/CVector3f.h>
#include <vector>

class CAnimation : public CResource
{
    DECLARE_RESOURCE_TYPE(eAnimation)
    friend class CAnimationLoader;

    typedef std::vector<CVector3f> TScaleChannel;
    typedef std::vector<CQuaternion> TRotationChannel;
    typedef std::vector<CVector3f> TTranslationChannel;

    float mDuration;
    float mTickInterval;
    u32 mNumKeys;

    std::vector<TScaleChannel> mScaleChannels;
    std::vector<TRotationChannel> mRotationChannels;
    std::vector<TTranslationChannel> mTranslationChannels;

    struct SBoneChannelInfo
    {
        u8 ScaleChannelIdx;
        u8 RotationChannelIdx;
        u8 TranslationChannelIdx;
    };
    SBoneChannelInfo mBoneInfo[100];

public:
    CAnimation(CResourceEntry *pEntry = 0);
    void EvaluateTransform(float Time, u32 BoneID, CVector3f *pOutTranslation, CQuaternion *pOutRotation, CVector3f *pOutScale) const;
    bool HasTranslation(u32 BoneID) const;

    inline float Duration() const       { return mDuration; }
    inline u32 NumKeys() const          { return mNumKeys; }
    inline float TickInterval() const   { return mTickInterval; }
};

#endif // CANIMATION_H
