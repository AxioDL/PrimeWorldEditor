#ifndef CINTPROPERTY_H
#define CINTPROPERTY_H

#include "../IPropertyNew.h"

class CIntProperty : public TNumericalPropertyNew< s32, EPropertyTypeNew::Int >
{
    friend class IPropertyNew;

protected:
    CIntProperty()
        : TNumericalPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (u32&) ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( Value(pData), 0, 10 );
    }
};

#endif // CINTPROPERTY_H
