#ifndef IMETAANIMATION
#define IMETAANIMATION

#include <Common/TString.h>
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CAnimation.h"

#include <memory>
#include <set>
#include <vector>

enum class EMetaAnimType
{
    Play = 0,
    Blend = 1,
    PhaseBlend = 2, // note: structure shared with eMAT_Blend
    Random = 3,
    Sequence = 4
};

// Factory class
class CMetaAnimFactory
{
public:
    static std::unique_ptr<class IMetaAnimation> LoadFromStream(CResourceStore* store, IInputStream& rInput, EGame Game);
};

// Animation primitive class
class CAnimPrimitive
{
    TResPtr<CAnimation> mpAnim;
    uint32_t mID = 0;
    TString mName;

public:
    CAnimPrimitive() = default;

    CAnimPrimitive(CResourceStore* store, const CAssetID& rkAnimAssetID, uint32_t CharAnimID, const TString& rkAnimName)
        : mID(CharAnimID), mName(rkAnimName)
    {
        mpAnim = store->LoadResource(rkAnimAssetID);
    }

    CAnimPrimitive(CResourceStore* store, IInputStream& rInput, EGame Game)
    {
        mpAnim = store->LoadResource(CAssetID(rInput, Game));
        mID = rInput.ReadU32();
        mName = rInput.ReadString();
    }

    bool operator==(const CAnimPrimitive& other) const { return mID == other.mID; }
    bool operator!=(const CAnimPrimitive& other) const { return !operator==(other); }
    bool operator< (const CAnimPrimitive& other) const { return mID < other.mID; }

    // Accessors
    CAnimation* Animation() const   { return mpAnim; }
    uint32_t ID() const             { return mID; }
    const TString& Name() const     { return mName; }
};

class CCharAnimTime
{
public:
    enum class EType
    {
        NonZero,
        ZeroIncreasing,
        ZeroSteady,
        ZeroDecreasing,
        Infinity,
    };

    CCharAnimTime() = default;
    explicit CCharAnimTime(float time, EType type) : mTime{time}, mType{type} {}

    float Seconds() const { return mTime; }
    EType Type() const { return mType; }

    void SetTime(float time) { mTime = time; }
    void SetType(EType type) { mType = type; }

private:
    float mTime{};
    EType mType{};
};

// Base MetaAnimation interface
class IMetaAnimation
{
public:
    virtual ~IMetaAnimation() = default;
    virtual EMetaAnimType Type() const = 0;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const = 0;
};

// CMetaAnimPlay - plays an animation
class CMetaAnimPlay : public IMetaAnimation
{
protected:
    CAnimPrimitive mPrimitive;
    CCharAnimTime mTime;

public:
    CMetaAnimPlay(CAnimPrimitive rkPrimitive, float time, CCharAnimTime::EType type);
    CMetaAnimPlay(CResourceStore* store, IInputStream& rInput, EGame Game);
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;

    // Accessors
    const CAnimPrimitive& Primitive() const { return mPrimitive; }
    float Seconds() const                   { return mTime.Seconds(); }
    CCharAnimTime::EType AnimType() const   { return mTime.Type(); }
};

// CMetaAnimBlend - blend between two animations
class CMetaAnimBlend : public IMetaAnimation
{
protected:
    EMetaAnimType mType{};
    std::unique_ptr<IMetaAnimation> mpMetaAnimA;
    std::unique_ptr<IMetaAnimation> mpMetaAnimB;
    float mBlend{};
    bool mUnknown{};

public:
    CMetaAnimBlend(CResourceStore* store, EMetaAnimType Type, IInputStream& rInput, EGame Game);
    ~CMetaAnimBlend() override;
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;

    // Accessors
    IMetaAnimation* BlendAnimationA() const  { return mpMetaAnimA.get(); }
    IMetaAnimation* BlendAnimationB() const  { return mpMetaAnimB.get(); }
    float Blend() const                      { return mBlend; }
    bool Unknown() const                     { return mUnknown; }
};

// SAnimProbabilityPair - structure used by CMetaAnimationRandom to associate an animation with a probability value
struct SAnimProbabilityPair
{
    std::unique_ptr<IMetaAnimation> pAnim;
    uint32_t Probability{};
};

// CMetaAnimRandom - play random animation
class CMetaAnimRandom : public IMetaAnimation
{
protected:
    std::vector<SAnimProbabilityPair> mProbabilityPairs;

public:
    CMetaAnimRandom(CResourceStore* store, IInputStream& rInput, EGame Game);
    ~CMetaAnimRandom() override;
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

// CMetaAnim - play a series of animations in sequence
class CMetaAnimSequence : public IMetaAnimation
{
protected:
    std::vector<std::unique_ptr<IMetaAnimation>> mAnimations;

public:
    CMetaAnimSequence(CResourceStore* store, IInputStream& rInput, EGame Game);
    ~CMetaAnimSequence() override;
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

#endif // IMETAANIMATION

