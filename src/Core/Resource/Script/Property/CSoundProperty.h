#ifndef CSOUNDPROPERTY_H
#define CSOUNDPROPERTY_H

#include "../IPropertyNew.h"

class CSoundProperty : public TSerializeableTypedProperty< s32, EPropertyTypeNew::Sound >
{
    friend class IPropertyNew;

protected:
    CSoundProperty()
        : TSerializeableTypedProperty()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( (u32&) ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData)
    {
        return TString::FromInt32( Value(pData), 0, 10 );
    }
};

#endif // CSOUNDPROPERTY_H
