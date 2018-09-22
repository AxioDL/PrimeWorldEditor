#ifndef CSTRINGPROPERTY_H
#define CSTRINGPROPERTY_H

#include "../IPropertyNew.h"

class CStringProperty : public TSerializeableTypedProperty< TString, EPropertyTypeNew::String >
{
    friend class IPropertyNew;

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
