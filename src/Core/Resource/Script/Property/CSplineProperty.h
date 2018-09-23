#ifndef CSPLINEPROPERTY_H
#define CSPLINEPROPERTY_H

#include "IProperty.h"

class CSplineProperty : public TTypedProperty< std::vector<char>, EPropertyType::Spline >
{
    friend class IProperty;

protected:
    CSplineProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc << SerialParameter("Data", ValueRef(pData));
    }
};

#endif // CSPLINEPROPERTY_H
