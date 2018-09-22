#ifndef CGUIDPROPERTY_H
#define CGUIDPROPERTY_H

#include "../IPropertyNew.h"

class CGuidProperty : public TTypedPropertyNew< std::vector<char>, EPropertyTypeNew::Guid >
{
    friend class IPropertyNew;

protected:
    CGuidProperty(EGame Game)
        : TTypedPropertyNew(Game)
    {}

public:
    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc << SerialParameter("Data", ValueRef(pData));
    }
};

#endif // CSPLINEPROPERTY_H
