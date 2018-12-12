#ifndef IMETATRANSITION_H
#define IMETATRANSITION_H

#include "IMetaAnimation.h"

class IMetaAnimation;
class IMetaTransition;

enum EMetaTransitionType
{
    eMTT_MetaAnim = 0,
    eMTT_Trans = 1,
    eMTT_PhaseTrans = 2, // note: structure shared with eMTT_Trans
    eMTT_Snap = 3,
    eMTT_Type4 = 4 // MP3 only
};

// Factory class
class CMetaTransFactory
{
public:
    class IMetaTransition* LoadFromStream(IInputStream& rInput, EGame Game);
};
extern CMetaTransFactory gMetaTransFactory;

// Base MetaTransition interface
class IMetaTransition
{
public:
    IMetaTransition() {}
    virtual ~IMetaTransition() {}
    virtual EMetaTransitionType Type() const = 0;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const = 0;
};

// CMetaTransMetaAnim
class CMetaTransMetaAnim : public IMetaTransition
{
    IMetaAnimation *mpAnim;

public:
    CMetaTransMetaAnim(IInputStream& rInput, EGame Game);
    ~CMetaTransMetaAnim();
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

// CMetaTransTrans
class CMetaTransTrans : public IMetaTransition
{
    EMetaTransitionType mType;
    float mUnknownA;
    uint32 mUnknownB;
    bool mUnknownC;
    bool mUnknownD;
    uint32 mUnknownE;

public:
    CMetaTransTrans(EMetaTransitionType Type, IInputStream& rInput, EGame Game);
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

// CMetaTransSnap
class CMetaTransSnap : public IMetaTransition
{
public:
    CMetaTransSnap(IInputStream& rInput, EGame Game);
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

// CMetaTransType4
class CMetaTransType4 : public IMetaTransition
{
public:
    CMetaTransType4(IInputStream& rInput, EGame Game);
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

#endif // IMETATRANSITION_H
