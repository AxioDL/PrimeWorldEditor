#ifndef CVECTORPROPERTY_H
#define CVECTORPROPERTY_H

#include "IProperty.h"

class CVectorProperty : public TSerializeableTypedProperty< CVector3f, EPropertyType::Vector >
{
    friend class IProperty;

protected:
    CVectorProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void PostInitialize() override
    {
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 0, "X");
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 4, "Y");
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 8, "Z");
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        ValueRef(pData).Serialize(Arc);
    }

    virtual TString ValueAsString(void* pData) const
    {
        return Value(pData).ToString();
    }
};

#endif // CVECTORPROPERTY_H
