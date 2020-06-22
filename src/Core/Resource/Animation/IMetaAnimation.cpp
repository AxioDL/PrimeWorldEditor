#include "IMetaAnimation.h"

// ************ CMetaAnimFactory ************
CMetaAnimFactory gMetaAnimFactory;

std::unique_ptr<IMetaAnimation> CMetaAnimFactory::LoadFromStream(IInputStream& rInput, EGame Game) const
{
    const auto Type = static_cast<EMetaAnimType>(rInput.ReadLong());

    switch (Type)
    {
    case EMetaAnimType::Play:
        return std::make_unique<CMetaAnimPlay>(rInput, Game);

    case EMetaAnimType::Blend:
    case EMetaAnimType::PhaseBlend:
        return std::make_unique<CMetaAnimBlend>(Type, rInput, Game);

    case EMetaAnimType::Random:
        return std::make_unique<CMetaAnimRandom>(rInput, Game);

    case EMetaAnimType::Sequence:
        return std::make_unique<CMetaAnimSequence>(rInput, Game);

    default:
        errorf("Unrecognized meta-animation type: %d", static_cast<int>(Type));
        return nullptr;
    }
}

// ************ CMetaAnimationPlay ************
CMetaAnimPlay::CMetaAnimPlay(const CAnimPrimitive& rkPrimitive, float UnkA, uint32 UnkB)
    : mPrimitive(rkPrimitive)
    , mUnknownA(UnkA)
    , mUnknownB(UnkB)
{
}

CMetaAnimPlay::CMetaAnimPlay(IInputStream& rInput, EGame Game)
{
    mPrimitive = CAnimPrimitive(rInput, Game);
    mUnknownA = rInput.ReadFloat();
    mUnknownB = rInput.ReadLong();
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
CMetaAnimBlend::CMetaAnimBlend(EMetaAnimType Type, IInputStream& rInput, EGame Game)
{
    ASSERT(Type == EMetaAnimType::Blend || Type == EMetaAnimType::PhaseBlend);
    mType = Type;
    mpMetaAnimA = gMetaAnimFactory.LoadFromStream(rInput, Game);
    mpMetaAnimB = gMetaAnimFactory.LoadFromStream(rInput, Game);
    mUnknownA = rInput.ReadFloat();
    mUnknownB = rInput.ReadBool();
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
CMetaAnimRandom::CMetaAnimRandom(IInputStream& rInput, EGame Game)
{
    const uint32 NumPairs = rInput.ReadLong();
    mProbabilityPairs.reserve(NumPairs);

    for (uint32 iAnim = 0; iAnim < NumPairs; iAnim++)
    {
        SAnimProbabilityPair Pair;
        Pair.pAnim = gMetaAnimFactory.LoadFromStream(rInput, Game);
        Pair.Probability = rInput.ReadLong();
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
CMetaAnimSequence::CMetaAnimSequence(IInputStream& rInput, EGame Game)
{
    const uint32 NumAnims = rInput.ReadLong();
    mAnimations.reserve(NumAnims);

    for (uint32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        mAnimations.push_back(gMetaAnimFactory.LoadFromStream(rInput, Game));
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
