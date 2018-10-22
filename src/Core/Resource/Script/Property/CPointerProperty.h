#ifndef CPOINTERPROPERTY_H
#define CPOINTERPROPERTY_H

#include "IProperty.h"

class CPointerProperty : public TTypedProperty<void*, EPropertyType::Pointer>
{
    friend class IProperty;

    CPointerProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    virtual bool IsPointerType() const
    {
        return true;
    }

    virtual void* GetChildDataPointer(void* pPropertyData) const
    {
        return ValueRef(pPropertyData);
    }

    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {
        // pointers are not serializable, this shouldn't happen
        ASSERT(false);
    }
};

#endif // CPOINTERPROPERTY_H
