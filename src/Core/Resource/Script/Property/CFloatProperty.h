#ifndef CFLOATPROPERTY_H
#define CFLOATPROPERTY_H

#include "IProperty.h"

class CFloatProperty : public TNumericalProperty< float, EPropertyType::Float >
{
    friend class IProperty;

protected:
    CFloatProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (float&) ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromFloat( Value(pData) );
    }
};

#endif // CFLOATPROPERTY_H
