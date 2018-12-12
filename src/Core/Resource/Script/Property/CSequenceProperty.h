#ifndef CSEQUENCEPROPERTY_H
#define CSEQUENCEPROPERTY_H

#include "IProperty.h"

class CSequenceProperty : public TTypedProperty< int32, EPropertyType::Sequence >
{
    friend class IProperty;

protected:
    CSequenceProperty(EGame Game)
        : TTypedProperty(Game)
    {}

    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {}
};

#endif // CSEQUENCEPROPERTY_H
