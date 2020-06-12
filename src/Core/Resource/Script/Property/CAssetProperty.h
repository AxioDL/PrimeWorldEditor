#ifndef CASSETPROPERTY_H
#define CASSETPROPERTY_H

#include "IProperty.h"
#include "Core/Resource/CResTypeFilter.h"

class CAssetProperty : public TSerializeableTypedProperty<CAssetID, EPropertyType::Asset>
{
    friend class IProperty;

    CResTypeFilter mTypeFilter;

protected:
    explicit CAssetProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {
        mDefaultValue = CAssetID::InvalidID( mGame );
    }

public:
    void Serialize(IArchive& rArc) override
    {
        TSerializeableTypedProperty::Serialize(rArc);
        CAssetProperty* pArchetype = static_cast<CAssetProperty*>(mpArchetype);
        rArc << SerialParameter("TypeFilter", mTypeFilter, pArchetype ? SH_Optional : 0, pArchetype ? pArchetype->mTypeFilter : CResTypeFilter());
    }

    bool ShouldSerialize() const override
    {
        CAssetProperty* pArchetype = static_cast<CAssetProperty*>(mpArchetype);
        return TSerializeableTypedProperty::ShouldSerialize() ||
                mTypeFilter != pArchetype->mTypeFilter;
    }

    void InitFromArchetype(IProperty* pOther) override
    {
        TTypedProperty::InitFromArchetype(pOther);
        mTypeFilter = static_cast<CAssetProperty*>(pOther)->mTypeFilter;
    }

    void SerializeValue(void* pData, IArchive& Arc) const override
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    TString ValueAsString(void* pData) const override
    {
        return Value(pData).ToString();
    }

    CAssetID GetSerializationDefaultValue() override
    {
        return CAssetID::InvalidID(Game());
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
