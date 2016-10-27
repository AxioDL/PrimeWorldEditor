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
    eMTT_Snap = 3
};

// Factory class
class CMetaTransFactory
{
public:
    class IMetaTransition* LoadFromStream(IInputStream& rInput);
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
    CMetaTransMetaAnim(IInputStream& rInput);
    ~CMetaTransMetaAnim();
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

// CMetaTransTrans
class CMetaTransTrans : public IMetaTransition
{
    EMetaTransitionType mType;
    float mUnknownA;
    u32 mUnknownB;
    bool mUnknownC;
    bool mUnknownD;
    u32 mUnknownE;

public:
    CMetaTransTrans(EMetaTransitionType Type, IInputStream& rInput);
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

// CMetaTransSnap
class CMetaTransSnap : public IMetaTransition
{
public:
    CMetaTransSnap(IInputStream& rInput);
    virtual EMetaTransitionType Type() const;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const;
};

#endif // IMETATRANSITION_H
