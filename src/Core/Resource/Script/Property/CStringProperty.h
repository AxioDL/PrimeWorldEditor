#ifndef CSTRINGPROPERTY_H
#define CSTRINGPROPERTY_H

#include "IProperty.h"

class CStringProperty : public TSerializeableTypedProperty< TString, EPropertyType::String >
{
    friend class IProperty;

protected:
    CStringProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return Value(pData);
    }
};

#endif // CSTRINGPROPERTY_H
