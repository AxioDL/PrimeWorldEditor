#ifndef IMETATRANSITION_H
#define IMETATRANSITION_H

#include "IMetaAnimation.h"
#include <memory>
#include <set>

class IInputStream;
class IMetaAnimation;
class IMetaTransition;
enum class EGame;

enum class EMetaTransType
{
    MetaAnim = 0,
    Trans = 1,
    PhaseTrans = 2, // note: structure shared with eMTT_Trans
    Snap = 3,
    Type4 = 4 // MP3 only
};

// Factory class
class CMetaTransFactory
{
public:
    std::unique_ptr<IMetaTransition> LoadFromStream(IInputStream& rInput, EGame Game) const;
};
extern CMetaTransFactory gMetaTransFactory;

// Base MetaTransition interface
class IMetaTransition
{
public:
    IMetaTransition() = default;
    virtual ~IMetaTransition() = default;
    virtual EMetaTransType Type() const = 0;
    virtual void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const = 0;
};

// CMetaTransMetaAnim
class CMetaTransMetaAnim : public IMetaTransition
{
    std::unique_ptr<IMetaAnimation> mpAnim;

public:
    CMetaTransMetaAnim(IInputStream& rInput, EGame Game);
    ~CMetaTransMetaAnim() override;
    EMetaTransType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

// CMetaTransTrans
class CMetaTransTrans : public IMetaTransition
{
    EMetaTransType mType;
    float mUnknownA = 0.0f;
    uint32 mUnknownB = 0;
    bool mUnknownC = false;
    bool mUnknownD = false;
    uint32 mUnknownE = 0;

public:
    CMetaTransTrans(EMetaTransType Type, IInputStream& rInput, EGame Game);
    EMetaTransType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

// CMetaTransSnap
class CMetaTransSnap : public IMetaTransition
{
public:
    CMetaTransSnap(IInputStream& rInput, EGame Game);
    EMetaTransType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

// CMetaTransType4
class CMetaTransType4 : public IMetaTransition
{
public:
    CMetaTransType4(IInputStream& rInput, EGame Game);
    EMetaTransType Type() const override;
    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const override;
};

#endif // IMETATRANSITION_H
