#ifndef CASSETPROPERTY_H
#define CASSETPROPERTY_H

#include "../IPropertyNew.h"
#include "Core/Resource/CResTypeFilter.h"

class CAssetProperty : public TSerializeableTypedProperty<CAssetID, EPropertyTypeNew::Asset>
{
    friend class CTemplateLoader;
    CResTypeFilter mTypeFilter;

public:
    virtual void PostInitialize()
    {
        // Init default value to an invalid ID depending on the game
        if (!mDefaultValue.IsValid())
        {
            mDefaultValue = CAssetID::InvalidID( mGame );
        }
    }

    virtual void Serialize(IArchive& rArc)
    {
        TSerializeableTypedProperty::Serialize(rArc);
        rArc << SerialParameter("TypeFilter", mTypeFilter);
    }

    virtual void InitFromArchetype(IPropertyNew* pOther)
    {
        TTypedPropertyNew::InitFromArchetype(pOther);
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
