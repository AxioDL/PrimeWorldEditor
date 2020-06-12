#ifndef CBYTEPROPERTY_H
#define CBYTEPROPERTY_H

#include "IProperty.h"

class CByteProperty : public TNumericalProperty<int8, EPropertyType::Byte>
{
    friend class IProperty;

protected:
    explicit CByteProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive((int8&)ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromInt32((int32)Value(pData), 0, 10);
    }
};

#endif // CBYTEPROPERTY_H
