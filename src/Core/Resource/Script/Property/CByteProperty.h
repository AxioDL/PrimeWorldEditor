#ifndef CBYTEPROPERTY_H
#define CBYTEPROPERTY_H

#include "IProperty.h"

class CByteProperty : public TNumericalProperty< s8, EPropertyType::Byte >
{
    friend class IProperty;

protected:
    CByteProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (u8&) ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( (s32) Value(pData), 0, 10 );
    }
};

#endif // CBYTEPROPERTY_H
