#ifndef CSHORTPROPERTY_H
#define CSHORTPROPERTY_H

#include "IProperty.h"

class CShortProperty : public TNumericalPropertyNew< s16, EPropertyTypeNew::Short >
{
    friend class IPropertyNew;

protected:
    CShortProperty(EGame Game)
        : TNumericalPropertyNew(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( (s32) Value(pData), 0, 10 );
    }
};

#endif // CSHORTPROPERTY_H
