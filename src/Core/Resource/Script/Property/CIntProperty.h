#ifndef CINTPROPERTY_H
#define CINTPROPERTY_H

#include "../IPropertyNew.h"

class CIntProperty : public TNumericalPropertyNew< s32, EPropertyTypeNew::Int >
{
    friend class IPropertyNew;

protected:
    CIntProperty(EGame Game)
        : TNumericalPropertyNew(Game)
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

#endif // CINTPROPERTY_H
