#ifndef CVECTORPROPERTY_H
#define CVECTORPROPERTY_H

#include "IProperty.h"

class CVectorProperty : public TSerializeableTypedProperty< CVector3f, EPropertyTypeNew::Vector >
{
    friend class IPropertyNew;

protected:
    CVectorProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void PostInitialize() override
    {
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 0, "X");
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 4, "Y");
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 8, "Z");
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
