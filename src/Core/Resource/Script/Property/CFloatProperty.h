#ifndef CFLOATPROPERTY_H
#define CFLOATPROPERTY_H

#include "IProperty.h"

class CFloatProperty : public TNumericalPropertyNew< float, EPropertyTypeNew::Float >
{
    friend class IPropertyNew;

protected:
    CFloatProperty(EGame Game)
        : TNumericalPropertyNew(Game)
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
