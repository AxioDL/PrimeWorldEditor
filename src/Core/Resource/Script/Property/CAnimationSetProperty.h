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
};

#endif // CANIMATIONSETPROPERTY_H
