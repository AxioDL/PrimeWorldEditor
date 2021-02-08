#ifndef CSEQUENCEPROPERTY_H
#define CSEQUENCEPROPERTY_H

#include "IProperty.h"

class CSequenceProperty : public TTypedProperty<int32, EPropertyType::Sequence>
{
    friend class IProperty;

protected:
    explicit CSequenceProperty(EGame Game)
        : TTypedProperty(Game)
    {}

    void SerializeValue(void* pData, IArchive& rArc) const override
    {}
};

#endif // CSEQUENCEPROPERTY_H
