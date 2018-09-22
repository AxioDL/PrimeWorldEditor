#ifndef CPOINTERPROPERTY_H
#define CPOINTERPROPERTY_H

#include "../IPropertyNew.h"

class CPointerProperty : public TTypedPropertyNew<void*, EPropertyTypeNew::Pointer>
{
    friend class CTemplateLoader;
    friend class IPropertyNew;

    CPointerProperty(EGame Game)
        : TTypedPropertyNew(Game)
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
