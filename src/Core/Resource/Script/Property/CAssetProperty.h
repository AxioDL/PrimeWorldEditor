#ifndef CASSETPROPERTY_H
#define CASSETPROPERTY_H

#include "IProperty.h"
#include "Core/Resource/CResTypeFilter.h"

class CAssetProperty : public TSerializeableTypedProperty<CAssetID, EPropertyType::Asset>
{
    friend class IProperty;

    CResTypeFilter mTypeFilter;

protected:
    CAssetProperty::CAssetProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
    {
        mDefaultValue = CAssetID::InvalidID( mGame );
    }

public:
    virtual void Serialize(IArchive& rArc)
    {
        TSerializeableTypedProperty::Serialize(rArc);
        CAssetProperty* pArchetype = static_cast<CAssetProperty*>(mpArchetype);
        rArc << SerialParameter("TypeFilter", mTypeFilter, pArchetype ? SH_Optional : 0, pArchetype ? pArchetype->mTypeFilter : CResTypeFilter());
    }

    virtual bool ShouldSerialize() const
    {
        CAssetProperty* pArchetype = static_cast<CAssetProperty*>(mpArchetype);
        return TSerializeableTypedProperty::ShouldSerialize() ||
                mTypeFilter != pArchetype->mTypeFilter;
    }

    virtual void InitFromArchetype(IProperty* pOther)
    {
        TTypedProperty::InitFromArchetype(pOther);
        mTypeFilter = static_cast<CAssetProperty*>(pOther)->mTypeFilter;
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        Arc.SerializePrimitive( ValueRef(pData), 0 );
    }

    virtual TString ValueAsString(void* pData) const
    {
        return Value(pData).ToString();
    }

    virtual CAssetID GetSerializationDefaultValue()
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
