#ifndef CBYTEPROPERTY_H
#define CBYTEPROPERTY_H

#include "../IPropertyNew.h"

class CByteProperty : public TNumericalPropertyNew< char, EPropertyTypeNew::Byte >
{
    friend class IPropertyNew;

protected:
    CByteProperty()
        : TNumericalPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (u8&) ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( (s32) Value(pData), 0, 10 );
    }
};

#endif // CBYTEPROPERTY_H
