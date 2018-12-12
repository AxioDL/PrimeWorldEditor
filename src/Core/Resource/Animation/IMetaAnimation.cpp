#include "IMetaAnimation.h"

// ************ CMetaAnimFactory ************
CMetaAnimFactory gMetaAnimFactory;

IMetaAnimation* CMetaAnimFactory::LoadFromStream(IInputStream& rInput, EGame Game)
{
    EMetaAnimationType Type = (EMetaAnimationType) rInput.ReadLong();

    switch (Type)
    {
    case eMAT_Play:
        return new CMetaAnimPlay(rInput, Game);

    case eMAT_Blend:
    case eMAT_PhaseBlend:
        return new CMetaAnimBlend(Type, rInput, Game);

    case eMAT_Random:
        return new CMetaAnimRandom(rInput, Game);

    case eMAT_Sequence:
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

EMetaAnimationType CMetaAnimPlay::Type() const
{
    return eMAT_Play;
}

void CMetaAnimPlay::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    rPrimSet.insert(mPrimitive);
}

// ************ CMetaAnimBlend ************
CMetaAnimBlend::CMetaAnimBlend(EMetaAnimationType Type, IInputStream& rInput, EGame Game)
{
    ASSERT(Type == eMAT_Blend || Type == eMAT_PhaseBlend);
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

EMetaAnimationType CMetaAnimBlend::Type() const
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

EMetaAnimationType CMetaAnimRandom::Type() const
{
    return eMAT_Random;
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

EMetaAnimationType CMetaAnimSequence::Type() const
{
    return eMAT_Sequence;
}

void CMetaAnimSequence::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    for (uint32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
        mAnimations[iAnim]->GetUniquePrimitives(rPrimSet);
}
