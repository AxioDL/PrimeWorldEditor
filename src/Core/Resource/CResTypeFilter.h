#ifndef CRESTYPEFILTER_H
#define CRESTYPEFILTER_H

#include "EResourceType.h"
#include "CResTypeInfo.h"
#include "Core/GameProject/CResourceEntry.h"

class CResTypeFilter
{
    EGame mGame;
    std::set<EResourceType> mAcceptedTypes;

public:
    CResTypeFilter() : mGame(EGame::Invalid) { }
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

    inline bool Accepts(EResourceType Type) const
    {
        return mAcceptedTypes.find(Type) != mAcceptedTypes.end();
    }

    inline bool Accepts(CResTypeInfo *pType) const
    {
        return pType && Accepts(pType->Type());
    }

    inline bool Accepts(CResourceEntry *pEntry) const
    {
        return pEntry && Accepts(pEntry->ResourceType());
    }

    inline bool Accepts(const CResTypeFilter& rkFilter) const
    {
        for (auto Iter = mAcceptedTypes.begin(); Iter != mAcceptedTypes.end(); Iter++)
        {
            if (rkFilter.Accepts(*Iter))
                return true;
        }

        return false;
    }

    inline bool operator==(const CResTypeFilter& rkOther) const
    {
        return mAcceptedTypes == rkOther.mAcceptedTypes;
    }

    inline bool operator!=(const CResTypeFilter& rkOther) const
    {
        return !(*this == rkOther);
    }
};

#endif // CRESTYPEFILTER_H
