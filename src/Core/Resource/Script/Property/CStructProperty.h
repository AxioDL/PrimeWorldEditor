#ifndef CSTRUCTPROPERTY_H
#define CSTRUCTPROPERTY_H

#include "../IPropertyNew.h"

class CStructPropertyNew : public IPropertyNew
{
    friend class CTemplateLoader;
    friend class IPropertyNew;

public:
    // Must be a valid type for TPropertyRef
    typedef void* ValueType;

protected:
    /** For archetypes, the filename of the template XML file. */
    TString mTemplateFileName;

    CStructPropertyNew(EGame Game)
        : IPropertyNew(Game)
    {}

public:
    virtual EPropertyTypeNew Type() const;
    virtual u32 DataSize() const;
    virtual u32 DataAlignment() const;
    virtual void Construct(void* pData) const;
    virtual void Destruct(void* pData) const;
    virtual bool MatchesDefault(void* pData) const;
    virtual void RevertToDefault(void* pData) const;
    virtual const char* HashableTypeName() const;
    virtual void Serialize(IArchive& rArc);
    virtual void SerializeValue(void* pData, IArchive& Arc) const;
    virtual void InitFromArchetype(IPropertyNew* pOther);
    virtual bool ShouldSerialize() const;
    virtual TString GetTemplateFileName();

    inline static EPropertyTypeNew StaticType() { return EPropertyTypeNew::Struct; }
};

#endif
