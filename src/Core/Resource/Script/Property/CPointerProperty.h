#ifndef CPOINTERPROPERTY_H
#define CPOINTERPROPERTY_H

#include "IProperty.h"

class CPointerProperty : public TTypedProperty<void*, EPropertyType::Pointer>
{
    friend class IProperty;

    explicit CPointerProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    bool IsPointerType() const override
    {
        return true;
    }

    void* GetChildDataPointer(void* pPropertyData) const override
    {
        return ValueRef(pPropertyData);
    }

    void SerializeValue(void* pData, IArchive& rArc) const override
    {
        // pointers are not serializable, this shouldn't happen
        ASSERT(false);
    }
};

#endif // CPOINTERPROPERTY_H
