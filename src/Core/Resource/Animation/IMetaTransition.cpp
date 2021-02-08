#include "IMetaTransition.h"
#include "IMetaAnimation.h"

// ************ CMetaTransFactory ************
CMetaTransFactory gMetaTransFactory;

std::unique_ptr<IMetaTransition> CMetaTransFactory::LoadFromStream(IInputStream& rInput, EGame Game) const
{
    const auto Type = static_cast<EMetaTransType>(rInput.ReadLong());

    switch (Type)
    {
    case EMetaTransType::MetaAnim:
        return std::make_unique<CMetaTransMetaAnim>(rInput, Game);

    case EMetaTransType::Trans:
    case EMetaTransType::PhaseTrans:
        return std::make_unique<CMetaTransTrans>(Type, rInput, Game);

    case EMetaTransType::Snap:
        return std::make_unique<CMetaTransSnap>(rInput, Game);

    case EMetaTransType::Type4:
        return std::make_unique<CMetaTransType4>(rInput, Game);

    default:
        errorf("Unrecognized meta-transition type: %d", static_cast<int>(Type));
        return nullptr;
    }
}

// ************ CMetaTransMetaAnim ************
CMetaTransMetaAnim::CMetaTransMetaAnim(IInputStream& rInput, EGame Game)
    : mpAnim{gMetaAnimFactory.LoadFromStream(rInput, Game)}
{
}

CMetaTransMetaAnim::~CMetaTransMetaAnim() = default;

EMetaTransType CMetaTransMetaAnim::Type() const
{
    return EMetaTransType::MetaAnim;
}

void CMetaTransMetaAnim::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    mpAnim->GetUniquePrimitives(rPrimSet);
}

// ************ CMetaTransTrans ************
CMetaTransTrans::CMetaTransTrans(EMetaTransType Type, IInputStream& rInput, EGame Game)
{
    ASSERT(Type == EMetaTransType::Trans || Type == EMetaTransType::PhaseTrans);
    mType = Type;

    if (Game <= EGame::Echoes)
    {
        mUnknownA = rInput.ReadFloat();
        mUnknownB = rInput.ReadLong();
        mUnknownC = rInput.ReadBool();
        mUnknownD = rInput.ReadBool();
        mUnknownE = rInput.ReadLong();
    }
    else
    {
        rInput.Skip(0x13);
    }
}

EMetaTransType CMetaTransTrans::Type() const
{
    return mType;
}

void CMetaTransTrans::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}

// ************ CMetaTransSnap ************
CMetaTransSnap::CMetaTransSnap(IInputStream&, EGame)
{
}

EMetaTransType CMetaTransSnap::Type() const
{
    return EMetaTransType::Snap;
}

void CMetaTransSnap::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}

// ************ CMetaTransType4 ************
CMetaTransType4::CMetaTransType4(IInputStream& rInput, EGame)
{
    rInput.Skip(0x14);
}

EMetaTransType CMetaTransType4::Type() const
{
    return EMetaTransType::Type4;
}

void CMetaTransType4::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}
