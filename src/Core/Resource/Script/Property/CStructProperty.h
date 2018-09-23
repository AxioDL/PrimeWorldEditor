#ifndef CSTRUCTPROPERTY_H
#define CSTRUCTPROPERTY_H

#include "IProperty.h"

class CStructProperty : public IProperty
{
    friend class CTemplateLoader;
    friend class IProperty;

public:
    // Must be a valid type for TPropertyRef
    typedef void* ValueType;

protected:
    /** For archetypes, the filename of the template XML file. */
    TString mTemplateFileName;

    CStructProperty(EGame Game)
        : IProperty(Game)
    {}

public:
    virtual EPropertyType Type() const;
    virtual u32 DataSize() const;
    virtual u32 DataAlignment() const;
    virtual void Construct(void* pData) const;
    virtual void Destruct(void* pData) const;
    virtual bool MatchesDefault(void* pData) const;
    virtual void RevertToDefault(void* pData) const;
    virtual const char* HashableTypeName() const;
    virtual void Serialize(IArchive& rArc);
    virtual void SerializeValue(void* pData, IArchive& Arc) const;
    virtual void InitFromArchetype(IProperty* pOther);
    virtual bool ShouldSerialize() const;
    virtual TString GetTemplateFileName();

    inline static EPropertyType StaticType() { return EPropertyType::Struct; }
};

#endif
