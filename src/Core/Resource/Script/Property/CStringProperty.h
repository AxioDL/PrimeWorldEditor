#ifndef CSTRINGPROPERTY_H
#define CSTRINGPROPERTY_H

#include "IProperty.h"

class CStringProperty : public TSerializeableTypedProperty<TString, EPropertyType::String>
{
    friend class IProperty;

protected:
    explicit CStringProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive(ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return Value(pData);
    }
};

#endif // CSTRINGPROPERTY_H
