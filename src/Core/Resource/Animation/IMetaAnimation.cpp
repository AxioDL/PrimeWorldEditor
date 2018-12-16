#include "IMetaAnimation.h"

// ************ CMetaAnimFactory ************
CMetaAnimFactory gMetaAnimFactory;

IMetaAnimation* CMetaAnimFactory::LoadFromStream(IInputStream& rInput, EGame Game)
{
    EMetaAnimType Type = (EMetaAnimType) rInput.ReadLong();

    switch (Type)
    {
    case EMetaAnimType::Play:
        return new CMetaAnimPlay(rInput, Game);

    case EMetaAnimType::Blend:
    case EMetaAnimType::PhaseBlend:
        return new CMetaAnimBlend(Type, rInput, Game);

    case EMetaAnimType::Random:
        return new CMetaAnimRandom(rInput, Game);

    case EMetaAnimType::Sequence:
        return new CMetaAnimSequence(rInput, Game);

    default:
        errorf("Unrecognized meta-animation type: %d", Type);
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

CMetaAnimBlend::~CMetaAnimBlend()
{
    delete mpMetaAnimA;
    delete mpMetaAnimB;
}

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
    uint32 NumPairs = rInput.ReadLong();
    mProbabilityPairs.reserve(NumPairs);

    for (uint32 iAnim = 0; iAnim < NumPairs; iAnim++)
    {
        SAnimProbabilityPair Pair;
        Pair.pAnim = gMetaAnimFactory.LoadFromStream(rInput, Game);
        Pair.Probability = rInput.ReadLong();
        mProbabilityPairs.push_back(Pair);
    }
}

CMetaAnimRandom::~CMetaAnimRandom()
{
    for (uint32 iPair = 0; iPair < mProbabilityPairs.size(); iPair++)
        delete mProbabilityPairs[iPair].pAnim;
}

EMetaAnimType CMetaAnimRandom::Type() const
{
    return EMetaAnimType::Random;
}

void CMetaAnimRandom::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    for (uint32 iPair = 0; iPair < mProbabilityPairs.size(); iPair++)
        mProbabilityPairs[iPair].pAnim->GetUniquePrimitives(rPrimSet);
}

// ************ CMetaAnimSequence ************
CMetaAnimSequence::CMetaAnimSequence(IInputStream& rInput, EGame Game)
{
    uint32 NumAnims = rInput.ReadLong();
    mAnimations.reserve(NumAnims);

    for (uint32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        IMetaAnimation *pAnim = gMetaAnimFactory.LoadFromStream(rInput, Game);
        mAnimations.push_back(pAnim);
    }
}

CMetaAnimSequence::~CMetaAnimSequence()
{
    for (uint32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
        delete mAnimations[iAnim];
}

EMetaAnimType CMetaAnimSequence::Type() const
{
    return EMetaAnimType::Sequence;
}

void CMetaAnimSequence::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    for (uint32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
        mAnimations[iAnim]->GetUniquePrimitives(rPrimSet);
}
