#ifndef CGUIDPROPERTY_H
#define CGUIDPROPERTY_H

#include "../IPropertyNew.h"

class CGuidProperty : public TTypedPropertyNew< std::vector<char>, EPropertyTypeNew::Guid >
{
    friend class IPropertyNew;

protected:
    CGuidProperty()
        : TTypedPropertyNew()
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializeBulkData( ValueRef(pData) );
    }
};

#endif // CSPLINEPROPERTY_H
