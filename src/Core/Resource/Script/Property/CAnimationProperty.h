#ifndef CANIMATIONPROPERTY_H
#define CANIMATIONPROPERTY_H

#include "IProperty.h"

class CAnimationProperty : public TSerializeableTypedProperty< uint32, EPropertyType::Animation >
{
    friend class IProperty;

protected:
    CAnimationProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {
        rArc.SerializePrimitive( (uint32&) ValueRef(pData), SH_HexDisplay );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::HexString( (uint32) Value(pData) );
    }
};

#endif // CANIMATIONPROPERTY_H
