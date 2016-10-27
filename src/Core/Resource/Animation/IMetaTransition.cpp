#include "IMetaTransition.h"
#include "IMetaAnimation.h"

// ************ CMetaTransFactory ************
CMetaTransFactory gMetaTransFactory;

IMetaTransition* CMetaTransFactory::LoadFromStream(IInputStream& rInput)
{
    EMetaTransitionType Type = (EMetaTransitionType) rInput.ReadLong();

    switch (Type)
    {
    case eMTT_MetaAnim:
        return new CMetaTransMetaAnim(rInput);

    case eMTT_Trans:
    case eMTT_PhaseTrans:
        return new CMetaTransTrans(Type, rInput);

    case eMTT_Snap:
        return new CMetaTransSnap(rInput);

    default:
        Log::Error("Unrecognized meta-transition type: " + TString::FromInt32(Type, 0, 10));
        return nullptr;
    }
}

// ************ CMetaTransMetaAnim ************
CMetaTransMetaAnim::CMetaTransMetaAnim(IInputStream& rInput)
{
    mpAnim = gMetaAnimFactory.LoadFromStream(rInput);
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
CMetaTransTrans::CMetaTransTrans(EMetaTransitionType Type, IInputStream& rInput)
{
    ASSERT(Type == eMTT_Trans || Type == eMTT_PhaseTrans);
    mType = Type;
    mUnknownA = rInput.ReadFloat();
    mUnknownB = rInput.ReadLong();
    mUnknownC = rInput.ReadBool();
    mUnknownD = rInput.ReadBool();
    mUnknownE = rInput.ReadLong();
}

EMetaTransitionType CMetaTransTrans::Type() const
{
    return mType;
}

void CMetaTransTrans::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}

// ************ CMetaTransSnap ************
CMetaTransSnap::CMetaTransSnap(IInputStream&)
{
}

EMetaTransitionType CMetaTransSnap::Type() const
{
    return eMTT_Snap;
}

void CMetaTransSnap::GetUniquePrimitives(std::set<CAnimPrimitive>&) const
{
}
