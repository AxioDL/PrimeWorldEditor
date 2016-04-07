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

    typedef std::vector<CQuaternion> TRotationChannel;
    typedef std::vector<CVector3f> TTranslationChannel;

    float mDuration;
    float mTickInterval;
    u32 mNumKeys;

    std::vector<TRotationChannel> mRotationChannels;
    std::vector<TTranslationChannel> mTranslationChannels;

    struct SBoneChannelInfo
    {
        u8 RotationChannelIdx;
        u8 TranslationChannelIdx;
    };
    std::vector<SBoneChannelInfo> mBoneInfo;

public:
    CAnimation();
    void EvaluateTransform(float Time, u32 BoneID, CTransform4f& rOut) const;
};

#endif // CANIMATION_H
