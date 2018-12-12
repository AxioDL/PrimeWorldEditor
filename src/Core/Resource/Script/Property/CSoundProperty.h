#ifndef CSOUNDPROPERTY_H
#define CSOUNDPROPERTY_H

#include "IProperty.h"

class CSoundProperty : public TSerializeableTypedProperty< int32, EPropertyType::Sound >
{
    friend class IProperty;

protected:
    CSoundProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return TString::FromInt32( Value(pData), 0, 10 );
    }
};

#endif // CSOUNDPROPERTY_H
