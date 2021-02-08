#ifndef CSHORTPROPERTY_H
#define CSHORTPROPERTY_H

#include "IProperty.h"

class CShortProperty : public TNumericalProperty< int16, EPropertyType::Short >
{
    friend class IProperty;

protected:
    explicit CShortProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive(ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromInt32((int32)Value(pData), 0, 10);
    }
};

#endif // CSHORTPROPERTY_H
