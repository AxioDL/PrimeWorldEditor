#ifndef CGUIDPROPERTY_H
#define CGUIDPROPERTY_H

#include "IProperty.h"

class CGuidProperty : public TTypedProperty< std::vector<char>, EPropertyType::Guid >
{
    friend class IProperty;

protected:
    CGuidProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc << SerialParameter("Data", ValueRef(pData));
    }
};

#endif // CSPLINEPROPERTY_H
