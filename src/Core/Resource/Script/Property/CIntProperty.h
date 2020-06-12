#ifndef CINTPROPERTY_H
#define CINTPROPERTY_H

#include "IProperty.h"

class CIntProperty : public TNumericalProperty<int32, EPropertyType::Int>
{
    friend class IProperty;

protected:
    explicit CIntProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive(ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromInt32(Value(pData), 0, 10);
    }
};

#endif // CINTPROPERTY_H
