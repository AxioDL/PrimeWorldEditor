#ifndef IMETAANIMATION
#define IMETAANIMATION

#include "CAnimation.h"
#include "Core/Resource/TResPtr.h"
#include <Common/TString.h>

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
    std::unique_ptr<class IMetaAnimation> LoadFromStream(IInputStream& rInput, EGame Game) const;
};
extern CMetaAnimFactory gMetaAnimFactory;

// Animation primitive class
class CAnimPrimitive
{
    TResPtr<CAnimation> mpAnim;
    uint32 mID = 0;
    TString mName;

public:
    CAnimPrimitive() = default;

    CAnimPrimitive(const CAssetID& rkAnimAssetID, uint32 CharAnimID, const TString& rkAnimName)
        : mID(CharAnimID), mName(rkAnimName)
    {
        mpAnim = gpResourceStore->LoadResource(rkAnimAssetID);
    }

    CAnimPrimitive(IInputStream& rInput, EGame Game)
    {
        mpAnim = gpResourceStore->LoadResource( CAssetID(rInput, Game) );
        mID = rInput.ReadLong();
        mName = rInput.ReadString();
    }

    bool operator==(const CAnimPrimitive& other) const { return mID == other.mID; }
    bool operator!=(const CAnimPrimitive& other) const { return !operator==(other); }
    bool operator< (const CAnimPrimitive& other) const { return mID < other.mID; }

    // Accessors
    CAnimation* Animation() const   { return mpAnim; }
    uint32 ID() const               { return mID; }
    TString Name() const            { return mName; }
};

// Base MetaAnimation interface
class IMetaAnimation
{
public:
    IMetaAnimation() = default;
    virtual ~IMetaAnimation() = default;
    virtual EMetaAnimType Type() const = 0;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const = 0;

    // Static
    static std::unique_ptr<IMetaAnimation> LoadFromStream(IInputStream& rInput, EGame Game);
};

// CMetaAnimPlay - plays an animation
class CMetaAnimPlay : public IMetaAnimation
{
protected:
    CAnimPrimitive mPrimitive;
    float mUnknownA;
    uint32 mUnknownB;

public:
    CMetaAnimPlay(const CAnimPrimitive& rkPrimitive, float UnkA, uint32 UnkB);
    CMetaAnimPlay(IInputStream& rInput, EGame Game);
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;

    // Accessors
    CAnimPrimitive Primitive() const { return mPrimitive; }
    float UnknownA() const           { return mUnknownA; }
    uint32 UnknownB() const          { return mUnknownB; }
};

// CMetaAnimBlend - blend between two animations
class CMetaAnimBlend : public IMetaAnimation
{
protected:
    EMetaAnimType mType;
    std::unique_ptr<IMetaAnimation> mpMetaAnimA;
    std::unique_ptr<IMetaAnimation> mpMetaAnimB;
    float mUnknownA;
    bool mUnknownB;

public:
    CMetaAnimBlend(EMetaAnimType Type, IInputStream& rInput, EGame Game);
    ~CMetaAnimBlend() override;
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;

    // Accessors
    IMetaAnimation* BlendAnimationA() const  { return mpMetaAnimA.get(); }
    IMetaAnimation* BlendAnimationB() const  { return mpMetaAnimB.get(); }
    float UnknownA() const                   { return mUnknownA; }
    bool UnknownB() const                    { return mUnknownB; }
};

// SAnimProbabilityPair - structure used by CMetaAnimationRandom to associate an animation with a probability value
struct SAnimProbabilityPair
{
    std::unique_ptr<IMetaAnimation> pAnim;
    uint32 Probability;
};

// CMetaAnimRandom - play random animation
class CMetaAnimRandom : public IMetaAnimation
{
protected:
    std::vector<SAnimProbabilityPair> mProbabilityPairs;

public:
    CMetaAnimRandom(IInputStream& rInput, EGame Game);
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
    CMetaAnimSequence(IInputStream& rInput, EGame Game);
    ~CMetaAnimSequence() override;
    EMetaAnimType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

#endif // IMETAANIMATION

