#ifndef CASSETPROPERTY_H
#define CASSETPROPERTY_H

#include "../IPropertyNew.h"
#include "Core/Resource/CResTypeFilter.h"

class CAssetProperty : public TTypedPropertyNew<CAssetID, EPropertyTypeNew::Asset>
{
    friend class CTemplateLoader;
    CResTypeFilter mTypeFilter;

public:
#if 0
    virtual void Serialize(IArchive& rArc)
    {
        TTypedPropertyNew::Serialize(rArc);
        rArc << SERIAL("AcceptedTypes", mTypeFilter);
    }
#endif

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
