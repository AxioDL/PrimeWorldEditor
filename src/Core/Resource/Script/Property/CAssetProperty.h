#ifndef CASSETPROPERTY_H
#define CASSETPROPERTY_H

#include "../IPropertyNew.h"
#include "Core/Resource/CResTypeFilter.h"

class CAssetProperty : public TSerializeableTypedProperty<CAssetID, EPropertyTypeNew::Asset>
{
    friend class CTemplateLoader;
    CResTypeFilter mTypeFilter;

public:
    virtual void Serialize(IArchive& rArc)
    {
        TSerializeableTypedProperty::Serialize(rArc);
        rArc << SERIAL("AcceptedTypes", mTypeFilter);
    }

    virtual void InitFromArchetype(IPropertyNew* pOther)
    {
        TTypedPropertyNew::InitFromArchetype(pOther);
        mTypeFilter = static_cast<CAssetProperty*>(pOther)->mTypeFilter;
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData) );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return Value(pData).ToString();
    }

    void SetTypeFilter(const TStringList& rkExtensions)
    {
        mTypeFilter.SetAcceptedTypes(Game(), rkExtensions);
    }

    const CResTypeFilter& GetTypeFilter() const
    {
        return mTypeFilter;
    }
};

#endif // CASSETPROPERTY_H
