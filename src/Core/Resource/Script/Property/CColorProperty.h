#ifndef CCOLORPROPERTY_H
#define CCOLORPROPERTY_H

#include "IProperty.h"
#include "CFloatProperty.h"

class CColorProperty : public TSerializeableTypedProperty<CColor, EPropertyType::Color>
{
    friend class IProperty;

protected:
    explicit CColorProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    void PostInitialize() override
    {
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 0,  "R");
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 4,  "G");
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 8,  "B");
        CreateIntrinsic(EPropertyType::Float, this, mOffset + 12, "A");
        TPropCast<CFloatProperty>( mChildren.back() )->SetDefaultValue(1.0f);
    }

    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        ValueRef(pData).Serialize(Arc);
    }
};

#endif // CVECTORPROPERTY_H
