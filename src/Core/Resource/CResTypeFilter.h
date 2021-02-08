#ifndef CRESTYPEFILTER_H
#define CRESTYPEFILTER_H

#include "EResType.h"
#include "CResTypeInfo.h"
#include "Core/GameProject/CResourceEntry.h"

#include <algorithm>

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

        for (const auto& str : rkTypes)
        {
            CResTypeInfo* pTypeInfo = CResTypeInfo::TypeForCookedExtension(mGame, CFourCC(str));

            if (pTypeInfo)
                mAcceptedTypes.insert(pTypeInfo->Type());
        }
    }

    TString ToString() const
    {
        TString Out;

        for (const auto type : mAcceptedTypes)
        {
            if (!Out.IsEmpty())
                Out += ',';

            CResTypeInfo *pTypeInfo = CResTypeInfo::FindTypeInfo(type);
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

    bool Accepts(const CResTypeInfo *pType) const
    {
        return pType != nullptr && Accepts(pType->Type());
    }

    bool Accepts(const CResourceEntry *pEntry) const
    {
        return pEntry != nullptr && Accepts(pEntry->ResourceType());
    }

    bool Accepts(const CResTypeFilter& filter) const
    {
        return std::any_of(mAcceptedTypes.cbegin(), mAcceptedTypes.cend(),
                           [&filter](const auto& entry) { return filter.Accepts(entry); });
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
