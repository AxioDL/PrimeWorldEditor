#ifndef CRESTYPEFILTER_H
#define CRESTYPEFILTER_H

#include "EResType.h"
#include "CResTypeInfo.h"
#include "Core/GameProject/CResourceEntry.h"

class CResTypeFilter
{
    EGame mGame = EGame::Invalid;
    std::set<EResourceType> mAcceptedTypes;

public:
    CResTypeFilter() = default;
    CResTypeFilter(EGame Game, const TString& rkTypeList) { FromString(Game, rkTypeList); }

    void SetAcceptedTypes(EGame Game, const TStringList& rkTypes)
    {
        mAcceptedTypes.clear();
        mGame = Game;

        for (auto Iter = rkTypes.begin(); Iter != rkTypes.end(); Iter++)
        {
            CResTypeInfo *pTypeInfo = CResTypeInfo::TypeForCookedExtension(mGame, CFourCC(*Iter));

            if (pTypeInfo)
                mAcceptedTypes.insert(pTypeInfo->Type());
        }
    }

    TString ToString() const
    {
        TString Out;

        for (auto Iter = mAcceptedTypes.begin(); Iter != mAcceptedTypes.end(); Iter++)
        {
            if (!Out.IsEmpty()) Out += ',';
            CResTypeInfo *pTypeInfo = CResTypeInfo::FindTypeInfo(*Iter);
            Out += pTypeInfo->CookedExtension(mGame).ToString();
        }

        return Out;
    }

    void FromString(EGame Game, const TString& rkString)
    {
        SetAcceptedTypes(Game, rkString.Split(","));
    }

    void Serialize(IArchive& rArc)
    {
        if (rArc.IsReader()) mGame = rArc.Game();
        rArc << SerialParameter("AcceptedTypes", mAcceptedTypes, SH_Proxy);
    }

    bool Accepts(EResourceType Type) const
    {
        return mAcceptedTypes.find(Type) != mAcceptedTypes.end();
    }

    bool Accepts(CResTypeInfo *pType) const
    {
        return pType && Accepts(pType->Type());
    }

    bool Accepts(CResourceEntry *pEntry) const
    {
        return pEntry && Accepts(pEntry->ResourceType());
    }

    bool Accepts(const CResTypeFilter& rkFilter) const
    {
        for (auto Iter = mAcceptedTypes.begin(); Iter != mAcceptedTypes.end(); Iter++)
        {
            if (rkFilter.Accepts(*Iter))
                return true;
        }

        return false;
    }

    bool operator==(const CResTypeFilter& rkOther) const
    {
        return mAcceptedTypes == rkOther.mAcceptedTypes;
    }

    bool operator!=(const CResTypeFilter& rkOther) const
    {
        return !(*this == rkOther);
    }
};

#endif // CRESTYPEFILTER_H
