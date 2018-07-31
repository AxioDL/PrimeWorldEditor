#ifndef CCOLORPROPERTY_H
#define CCOLORPROPERTY_H

#include "../IPropertyNew.h"

class CColorProperty : public TSerializeableTypedProperty< CColor, EPropertyTypeNew::Color >
{
    friend class IPropertyNew;

protected:
    CColorProperty()
        : TSerializeableTypedProperty()
    {}

public:
    virtual void PostInitialize()
    {
        IPropertyNew* pR = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        IPropertyNew* pG = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        IPropertyNew* pB = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        IPropertyNew* pA = Create(EPropertyTypeNew::Float, this, mGame, mpScriptTemplate);
        pR->SetName("R");
        pG->SetName("G");
        pB->SetName("B");
        pA->SetName("A");
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Value(pData).Serialize(Arc);
    }
};

#endif // CVECTORPROPERTY_H
