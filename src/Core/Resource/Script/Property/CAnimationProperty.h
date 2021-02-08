#ifndef CANIMATIONPROPERTY_H
#define CANIMATIONPROPERTY_H

#include "IProperty.h"

class CAnimationProperty : public TSerializeableTypedProperty<uint32, EPropertyType::Animation>
{
    friend class IProperty;

protected:
    explicit CAnimationProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& rArc) const override
    {
        rArc.SerializePrimitive((uint32&)ValueRef(pData), SH_HexDisplay);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::HexString(static_cast<uint32>(Value(pData)));
    }
};

#endif // CANIMATIONPROPERTY_H
