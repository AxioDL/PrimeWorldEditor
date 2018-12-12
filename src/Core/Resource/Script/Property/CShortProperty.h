#ifndef CSHORTPROPERTY_H
#define CSHORTPROPERTY_H

#include "IProperty.h"

class CShortProperty : public TNumericalProperty< int16, EPropertyType::Short >
{
    friend class IProperty;

protected:
    CShortProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( (int32) Value(pData), 0, 10 );
    }
};

#endif // CSHORTPROPERTY_H
