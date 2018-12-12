#ifndef CINTPROPERTY_H
#define CINTPROPERTY_H

#include "IProperty.h"

class CIntProperty : public TNumericalProperty< int32, EPropertyType::Int >
{
    friend class IProperty;

protected:
    CIntProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( Value(pData), 0, 10 );
    }
};

#endif // CINTPROPERTY_H
