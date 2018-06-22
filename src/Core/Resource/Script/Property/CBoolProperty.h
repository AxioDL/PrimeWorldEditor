#ifndef CBOOLPROPERTY_H
#define CBOOLPROPERTY_H

#include "../IPropertyNew.h"

class CBoolProperty : public TTypedPropertyNew< bool, EPropertyTypeNew::Bool >
{
    friend class IPropertyNew;

protected:
    CBoolProperty()
        : TTypedPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData)
    {
        return Value(pData) ? "true" : "false";
    }
};

#endif // CBOOLPROPERTY_H
