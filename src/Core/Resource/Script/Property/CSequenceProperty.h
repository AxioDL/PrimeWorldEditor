#ifndef CSEQUENCEPROPERTY_H
#define CSEQUENCEPROPERTY_H

#include "../IPropertyNew.h"

class CSequenceProperty : public TTypedPropertyNew< int, EPropertyTypeNew::Sequence >
{
    friend class IPropertyNew;

protected:
    CSequenceProperty()
        : TTypedPropertyNew()
    {}

    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {}
};

#endif // CSEQUENCEPROPERTY_H
