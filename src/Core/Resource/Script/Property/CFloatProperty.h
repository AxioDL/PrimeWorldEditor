#ifndef CFLOATPROPERTY_H
#define CFLOATPROPERTY_H

#include "IProperty.h"

class CFloatProperty : public TNumericalProperty<float, EPropertyType::Float>
{
    friend class IProperty;

protected:
    explicit CFloatProperty(EGame Game)
        : TNumericalProperty(Game)
    {}

public:
    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive((float&)ValueRef(pData), 0);
    }

    TString ValueAsString(void* pData) const override
    {
        return TString::FromFloat(Value(pData));
    }
};

#endif // CFLOATPROPERTY_H
