#ifndef CVECTORPROPERTY_H
#define CVECTORPROPERTY_H

#include "../IPropertyNew.h"

class CVectorProperty : public TSerializeableTypedProperty< CVector3f, EPropertyTypeNew::Vector >
{
    friend class IPropertyNew;

protected:
    CVectorProperty()
        : TSerializeableTypedProperty()
    {}

public:
    virtual void PostInitialize()
    {
        IPropertyNew* pX = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        IPropertyNew* pY = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        IPropertyNew* pZ = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        pX->SetName("X");
        pY->SetName("Y");
        pZ->SetName("Z");
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
