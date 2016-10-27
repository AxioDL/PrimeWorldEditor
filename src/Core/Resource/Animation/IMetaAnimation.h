#ifndef IMETAANIMATION
#define IMETAANIMATION

#include "CAnimation.h"
#include "Core/Resource/TResPtr.h"
#include <Common/TString.h>

enum EMetaAnimationType
{
    eMAT_Play = 0,
    eMAT_Blend = 1,
    eMAT_PhaseBlend = 2, // note: structure shared with eMAT_Blend, differences are currently unknown
    eMAT_Random = 3,
    eMAT_Sequence = 4
};

// Base MetaAnimation interface
class IMetaAnimation
{
protected:
    TString mName;
    EMetaAnimationType mType;

public:
    IMetaAnimation(EMetaAnimationType Type)
        : mType(Type) {}

    virtual ~IMetaAnimation() {}

    // Accessors
    inline void SetName(const TString& rkName)  { mName = rkName; }
    inline TString Name() const                 { return mName; }
    inline EMetaAnimationType Type() const      { return mType; }
};

// CMetaAnimationPlay - plays an animation
class CMetaAnimationPlay : public IMetaAnimation
{
protected:
    TResPtr<CAnimation> mpAnim;

public:
    CMetaAnimationPlay(CAnimation *pAnim)
        : IMetaAnimation(eMAT_Play), mpAnim(pAnim) {}

    inline CAnimation* GetPlayAnimation() const { return mpAnim; }
};

// CMetaAnimationBlend - blend between two animations
class CMetaAnimationBlend : public IMetaAnimation
{
protected:
    IMetaAnimation *mpAnimA;
    IMetaAnimation *mpAnimB;

public:
    CMetaAnimationBlend(IMetaAnimation *pAnimA, IMetaAnimation *pAnimB)
        : IMetaAnimation(eMAT_Blend), mpAnimA(pAnimA), mpAnimB(pAnimB) {}

    ~CMetaAnimationBlend()
    {
        delete mpAnimA;
        delete mpAnimB;
    }

    inline IMetaAnimation* BlendAnimationA() const  { return mpAnimA; }
    inline IMetaAnimation* BlendAnimationB() const  { return mpAnimB; }
};

// SAnimProbabilityPair - structure used by CMetaAnimationRandom to associate an animation with a probability value
struct SAnimProbabilityPair
{
    IMetaAnimation *pAnim;
    u32 Probability;
};

// CMetaAnimationRandom - play random animation
class CMetaAnimationRandom : public IMetaAnimation
{
protected:
    std::vector<SAnimProbabilityPair> mProbabilityPairs;

public:

};

#endif // IMETAANIMATION

