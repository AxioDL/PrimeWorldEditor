#ifndef CSPLINEPROPERTY_H
#define CSPLINEPROPERTY_H

#include "IProperty.h"

class CSplineProperty : public TTypedPropertyNew< std::vector<char>, EPropertyTypeNew::Spline >
{
    friend class IPropertyNew;

protected:
    CSplineProperty(EGame Game)
        : TTypedPropertyNew(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc << SerialParameter("Data", ValueRef(pData));
    }
};

#endif // CSPLINEPROPERTY_H
