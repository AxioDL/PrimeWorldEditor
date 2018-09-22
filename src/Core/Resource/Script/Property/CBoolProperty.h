#ifndef CBOOLPROPERTY_H
#define CBOOLPROPERTY_H

#include "IProperty.h"

class CBoolProperty : public TSerializeableTypedProperty< bool, EPropertyType::Bool >
{
    friend class IProperty;

protected:
    CBoolProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData)
    {
        return Value(pData) ? "true" : "false";
    }
};

#endif // CBOOLPROPERTY_H
