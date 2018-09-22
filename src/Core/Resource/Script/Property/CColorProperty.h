#ifndef CCOLORPROPERTY_H
#define CCOLORPROPERTY_H

#include "../IPropertyNew.h"
#include "CFloatProperty.h"

class CColorProperty : public TSerializeableTypedProperty< CColor, EPropertyTypeNew::Color >
{
    friend class IPropertyNew;

protected:
    CColorProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void PostInitialize()
    {
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 0,  "R");
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 4,  "G");
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 8,  "B");
        CreateIntrinsic(EPropertyTypeNew::Float, this, mOffset + 12, "A");
        TPropCast<CFloatProperty>( mChildren.back() )->SetDefaultValue(1.0f);
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Value(pData).Serialize(Arc);
    }
};

#endif // CVECTORPROPERTY_H
