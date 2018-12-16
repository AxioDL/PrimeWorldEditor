#ifndef CANIMATION_H
#define CANIMATION_H

#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CAnimEventData.h"
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CVector3f.h>
#include <vector>

class CAnimation : public CResource
{
    DECLARE_RESOURCE_TYPE(Animation)
    friend class CAnimationLoader;

    typedef std::vector<CVector3f> TScaleChannel;
    typedef std::vector<CQuaternion> TRotationChannel;
    typedef std::vector<CVector3f> TTranslationChannel;

    float mDuration;
    float mTickInterval;
    uint32 mNumKeys;

    std::vector<TScaleChannel> mScaleChannels;
    std::vector<TRotationChannel> mRotationChannels;
    std::vector<TTranslationChannel> mTranslationChannels;

    struct SBoneChannelInfo
    {
        uint8 ScaleChannelIdx;
        uint8 RotationChannelIdx;
        uint8 TranslationChannelIdx;
    };
    SBoneChannelInfo mBoneInfo[100];

    TResPtr<CAnimEventData> mpEventData;

public:
    CAnimation(CResourceEntry *pEntry = 0);
    CDependencyTree* BuildDependencyTree() const;
    void EvaluateTransform(float Time, uint32 BoneID, CVector3f *pOutTranslation, CQuaternion *pOutRotation, CVector3f *pOutScale) const;
    bool HasTranslation(uint32 BoneID) const;

    inline float Duration() const               { return mDuration; }
    inline uint32 NumKeys() const               { return mNumKeys; }
    inline float TickInterval() const           { return mTickInterval; }
    inline CAnimEventData* EventData() const    { return mpEventData; }
};

#endif // CANIMATION_H
