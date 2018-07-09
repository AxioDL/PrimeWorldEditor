#ifndef CSHORTPROPERTY_H
#define CSHORTPROPERTY_H

#include "../IPropertyNew.h"

class CShortProperty : public TNumericalPropertyNew< short, EPropertyTypeNew::Short >
{
    friend class IPropertyNew;

protected:
    CShortProperty()
        : TNumericalPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (u16&) ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( (s32) Value(pData), 0, 10 );
    }
};

#endif // CSHORTPROPERTY_H
