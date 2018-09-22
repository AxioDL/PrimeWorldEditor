#ifndef CSEQUENCEPROPERTY_H
#define CSEQUENCEPROPERTY_H

#include "IProperty.h"

class CSequenceProperty : public TTypedPropertyNew< s32, EPropertyTypeNew::Sequence >
{
    friend class IPropertyNew;

protected:
    CSequenceProperty(EGame Game)
        : TTypedPropertyNew(Game)
    {}

    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {}
};

#endif // CSEQUENCEPROPERTY_H
