#ifndef CSTRUCTPROPERTY_H
#define CSTRUCTPROPERTY_H

#include "IProperty.h"

class CStructProperty : public IProperty
{
    friend class IProperty;

public:
    // Must be a valid type for TPropertyRef
    using ValueType = void*;

protected:
    explicit CStructProperty(EGame Game)
        : IProperty(Game)
    {}

public:
    EPropertyType Type() const override;
    void PostInitialize() override;
    uint32 DataSize() const override;
    uint32 DataAlignment() const override;
    void Construct(void* pData) const override;
    void Destruct(void* pData) const override;
    bool MatchesDefault(void* pData) const override;
    void RevertToDefault(void* pData) const override;
    void SetDefaultFromData(void* pData) override;
    const char* HashableTypeName() const override;
    void Serialize(IArchive& rArc) override;
    void SerializeValue(void* pData, IArchive& Arc) const override;
    void InitFromArchetype(IProperty* pOther) override;
    bool ShouldSerialize() const override;

    static constexpr EPropertyType StaticType() { return EPropertyType::Struct; }
};

#endif
