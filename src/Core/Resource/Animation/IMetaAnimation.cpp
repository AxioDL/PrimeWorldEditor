#include "Core/Resource/Animation/IMetaAnimation.h"

#include <Common/Log.h>

// ************ CMetaAnimFactory ************

std::unique_ptr<IMetaAnimation> CMetaAnimFactory::LoadFromStream(CResourceStore* store, IInputStream& rInput, EGame Game)
{
    const auto Type = static_cast<EMetaAnimType>(rInput.ReadU32());

    switch (Type)
    {
    case EMetaAnimType::Play:
        return std::make_unique<CMetaAnimPlay>(store, rInput, Game);

    case EMetaAnimType::Blend:
    case EMetaAnimType::PhaseBlend:
        return std::make_unique<CMetaAnimBlend>(store, Type, rInput, Game);

    case EMetaAnimType::Random:
        return std::make_unique<CMetaAnimRandom>(store, rInput, Game);

    case EMetaAnimType::Sequence:
        return std::make_unique<CMetaAnimSequence>(store, rInput, Game);

    default:
        NLog::Error("Unrecognized meta-animation type: {}", static_cast<int>(Type));
        return nullptr;
    }
}

// ************ CMetaAnimationPlay ************
CMetaAnimPlay::CMetaAnimPlay(CAnimPrimitive rkPrimitive, float time, CCharAnimTime::EType type)
    : mPrimitive(std::move(rkPrimitive))
    , mTime(time, type)
{
}

CMetaAnimPlay::CMetaAnimPlay(CResourceStore* store, IInputStream& rInput, EGame Game)
    : mPrimitive(store, rInput, Game)
{
    mTime.SetTime(rInput.ReadF32());
    mTime.SetType(static_cast<CCharAnimTime::EType>(rInput.ReadU32()));
}

EMetaAnimType CMetaAnimPlay::Type() const
{
    return EMetaAnimType::Play;
}

void CMetaAnimPlay::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    rPrimSet.insert(mPrimitive);
}

// ************ CMetaAnimBlend ************
CMetaAnimBlend::CMetaAnimBlend(CResourceStore* store, EMetaAnimType Type, IInputStream& rInput, EGame Game)
{
    ASSERT(Type == EMetaAnimType::Blend || Type == EMetaAnimType::PhaseBlend);
    mType = Type;
    mpMetaAnimA = CMetaAnimFactory::LoadFromStream(store, rInput, Game);
    mpMetaAnimB = CMetaAnimFactory::LoadFromStream(store, rInput, Game);
    mBlend = rInput.ReadF32();
    mUnknown = rInput.ReadBool();
}

CMetaAnimBlend::~CMetaAnimBlend() = default;

EMetaAnimType CMetaAnimBlend::Type() const
{
    return mType;
}

void CMetaAnimBlend::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    mpMetaAnimA->GetUniquePrimitives(rPrimSet);
    mpMetaAnimB->GetUniquePrimitives(rPrimSet);
}

// ************ CMetaAnimRandom ************
CMetaAnimRandom::CMetaAnimRandom(CResourceStore* store, IInputStream& rInput, EGame Game)
{
    const auto NumPairs = rInput.ReadU32();
    mProbabilityPairs.reserve(NumPairs);

    for (uint32_t iAnim = 0; iAnim < NumPairs; iAnim++)
    {
        SAnimProbabilityPair Pair;
        Pair.pAnim = CMetaAnimFactory::LoadFromStream(store, rInput, Game);
        Pair.Probability = rInput.ReadU32();
        mProbabilityPairs.push_back(std::move(Pair));
    }
}

CMetaAnimRandom::~CMetaAnimRandom() = default;

EMetaAnimType CMetaAnimRandom::Type() const
{
    return EMetaAnimType::Random;
}

void CMetaAnimRandom::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    for (const auto& pair : mProbabilityPairs)
        pair.pAnim->GetUniquePrimitives(rPrimSet);
}

// ************ CMetaAnimSequence ************
CMetaAnimSequence::CMetaAnimSequence(CResourceStore* store, IInputStream& rInput, EGame Game)
{
    const auto NumAnims = rInput.ReadU32();
    mAnimations.reserve(NumAnims);

    for (uint32_t iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        mAnimations.push_back(CMetaAnimFactory::LoadFromStream(store, rInput, Game));
    }
}

CMetaAnimSequence::~CMetaAnimSequence() = default;

EMetaAnimType CMetaAnimSequence::Type() const
{
    return EMetaAnimType::Sequence;
}

void CMetaAnimSequence::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    for (const auto& anim : mAnimations)
        anim->GetUniquePrimitives(rPrimSet);
}
