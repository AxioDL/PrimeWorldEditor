#ifndef CANIMATION_H
#define CANIMATION_H

#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CAnimEventData.h"
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CVector3f.h>
#include <array>
#include <vector>

class CAnimation : public CResource
{
    DECLARE_RESOURCE_TYPE(Animation)
    friend class CAnimationLoader;

    using TScaleChannel = std::vector<CVector3f>;
    using TRotationChannel = std::vector<CQuaternion>;
    using TTranslationChannel = std::vector<CVector3f>;

    float mDuration = 0.0f;
    float mTickInterval = 0.0333333f;
    uint32 mNumKeys = 0;

    std::vector<TScaleChannel> mScaleChannels;
    std::vector<TRotationChannel> mRotationChannels;
    std::vector<TTranslationChannel> mTranslationChannels;

    struct SBoneChannelInfo
    {
        uint8 ScaleChannelIdx = 0xFF;
        uint8 RotationChannelIdx = 0xFF;
        uint8 TranslationChannelIdx = 0xFF;
    };
    std::array<SBoneChannelInfo, 100> mBoneInfo;

    TResPtr<CAnimEventData> mpEventData;

public:
    explicit CAnimation(CResourceEntry *pEntry = nullptr);
    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override;
    void EvaluateTransform(float Time, uint32 BoneID, CVector3f *pOutTranslation, CQuaternion *pOutRotation, CVector3f *pOutScale) const;
    bool HasTranslation(uint32 BoneID) const;

    float Duration() const               { return mDuration; }
    uint32 NumKeys() const               { return mNumKeys; }
    float TickInterval() const           { return mTickInterval; }
    CAnimEventData* EventData() const    { return mpEventData; }
};

#endif // CANIMATION_H
