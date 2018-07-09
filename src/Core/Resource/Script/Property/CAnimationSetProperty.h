#ifndef CANIMATIONSETPROPERTY_H
#define CANIMATIONSETPROPERTY_H

#include "../IPropertyNew.h"

class CAnimationSetProperty : public TTypedPropertyNew< CAnimationParameters, EPropertyTypeNew::AnimationSet >
{
    friend class IPropertyNew;

protected:
    CAnimationSetProperty()
        : TTypedPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Value(pData).Serialize(Arc);
    }

    virtual const char* HashableTypeName() const
    {
        return (Game() <= eEchoes ? "AnimationSet" : "CharacterAnimationSet");
    }
};

#endif // CANIMATIONSETPROPERTY_H
