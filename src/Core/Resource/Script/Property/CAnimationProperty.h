#ifndef CANIMATIONPROPERTY_H
#define CANIMATIONPROPERTY_H

#include "../IPropertyNew.h"

class CAnimationProperty : public TSerializeableTypedProperty< u32, EPropertyTypeNew::Animation >
{
    friend class IPropertyNew;

protected:
    CAnimationProperty()
        : TSerializeableTypedProperty()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {
        rArc.SerializeHexPrimitive( (u32&) ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::HexString( (u32) Value(pData) );
    }
};

#endif // CANIMATIONPROPERTY_H
