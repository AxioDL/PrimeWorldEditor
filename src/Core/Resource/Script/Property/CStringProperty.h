#ifndef CSTRINGPROPERTY_H
#define CSTRINGPROPERTY_H

#include "../IPropertyNew.h"

class CStringProperty : public TTypedPropertyNew< TString, EPropertyTypeNew::String >
{
    friend class IPropertyNew;

protected:
    CStringProperty()
        : TTypedPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return Value(pData);
    }
};

#endif // CSTRINGPROPERTY_H
