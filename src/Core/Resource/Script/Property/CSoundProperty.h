#ifndef CSOUNDPROPERTY_H
#define CSOUNDPROPERTY_H

#include "IProperty.h"

class CSoundProperty : public TSerializeableTypedProperty<int32, EPropertyType::Sound>
{
    friend class IProperty;

protected:
    explicit CSoundProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive(ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromInt32(Value(pData), 0, 10);
    }
};

#endif // CSOUNDPROPERTY_H
