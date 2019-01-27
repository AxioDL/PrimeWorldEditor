#ifndef CSTRUCTPROPERTY_H
#define CSTRUCTPROPERTY_H

#include "IProperty.h"

class CStructProperty : public IProperty
{
    friend class IProperty;

public:
    // Must be a valid type for TPropertyRef
    typedef void* ValueType;

protected:
    CStructProperty(EGame Game)
        : IProperty(Game)
    {}

public:
    virtual EPropertyType Type() const;
    virtual void PostInitialize();
    virtual uint32 DataSize() const;
    virtual uint32 DataAlignment() const;
    virtual void Construct(void* pData) const;
    virtual void Destruct(void* pData) const;
    virtual bool MatchesDefault(void* pData) const;
    virtual void RevertToDefault(void* pData) const;
    virtual void SetDefaultFromData(void* pData);
    virtual const char* HashableTypeName() const;
    virtual void Serialize(IArchive& rArc);
    virtual void SerializeValue(void* pData, IArchive& Arc) const;
    virtual void InitFromArchetype(IProperty* pOther);
    virtual bool ShouldSerialize() const;

    inline static EPropertyType StaticType() { return EPropertyType::Struct; }
};

#endif
