#include "IMetaTransition.h"
#include "IMetaAnimation.h"

// ************ CMetaTransFactory ************
CMetaTransFactory gMetaTransFactory;

IMetaTransition* CMetaTransFactory::LoadFromStream(IInputStream& rInput, EGame Game)
{
    EMetaTransitionType Type = (EMetaTransitionType) rInput.ReadLong();

    switch (Type)
    {
    case eMTT_MetaAnim:
        return new CMetaTransMetaAnim(rInput, Game);

    case eMTT_Trans:
    case eMTT_PhaseTrans:
        return new CMetaTransTrans(Type, rInput, Game);

    case eMTT_Snap:
        return new CMetaTransSnap(rInput, Game);

    case eMTT_Type4:
        return new CMetaTransType4(rInput, Game);

    default:
        Log::Error("Unrecognized meta-transition type: " + TString::FromInt32(Type, 0, 10));
        return nullptr;
    }
}

// ************ CMetaTransMetaAnim ************
CMetaTransMetaAnim::CMetaTransMetaAnim(IInputStream& rInput, EGame Game)
{
    mpAnim = gMetaAnimFactory.LoadFromStream(rInput, Game);
}

CMetaTransMetaAnim::~CMetaTransMetaAnim()
{
    delete mpAnim;
}

EMetaTransitionType CMetaTransMetaAnim::Type() const
{
    return eMTT_MetaAnim;
}

void CMetaTransMetaAnim::GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
{
    mpAnim->GetUniquePrimitives(rPrimSet);
}

// ************ CMetaTransTrans ************
CMetaTransTrans::CMetaTransTrans(EMetaTransitionType Type, IInputStream& rInput, EGame Game)
{
    ASSERT(Type == eMTT_Trans || Type == eMTT_PhaseTrans);
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

EMetaTransitionType CMetaTransTrans::Type() const
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

EMetaTransitionType CMetaTransSnap::Type() const
{
    return eMTT_Snap;
}

void CMetaTransSnap::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}

// ************ CMetaTransType4 ************
CMetaTransType4::CMetaTransType4(IInputStream& rInput, EGame)
{
    rInput.Skip(0x14);
}

EMetaTransitionType CMetaTransType4::Type() const
{
    return eMTT_Type4;
}

void CMetaTransType4::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}
