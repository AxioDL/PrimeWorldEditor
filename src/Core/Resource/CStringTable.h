#ifndef CSTRINGTABLE_H
#define CSTRINGTABLE_H

#include "CResource.h"
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <vector>

class CStringTable : public CResource
{
    DECLARE_RESOURCE_TYPE(eStringTable)
    friend class CStringLoader;

    std::vector<TString> mStringNames;
    u32 mNumStrings;

    struct SLangTable
    {
        CFourCC Language;
        std::vector<TWideString> Strings;
    };
    std::vector<SLangTable> mLangTables;

public:
    CStringTable(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    inline u32 NumStrings() const               { return mNumStrings; }
    inline u32 NumLanguages() const             { return mLangTables.size(); }
    inline CFourCC LanguageTag(u32 Index) const { return mLangTables[Index].Language; }
    inline TWideString String(u32 LangIndex, u32 StringIndex) const    { return mLangTables[LangIndex].Strings[StringIndex]; }
    inline TString StringName(u32 StringIndex) const                   { return mStringNames[StringIndex]; }

    TWideString String(CFourCC Lang, u32 StringIndex) const
    {
        for (u32 iLang = 0; iLang < NumLanguages(); iLang++)
        {
            if (LanguageTag(iLang) == Lang)
                return String(iLang, StringIndex);
        }

        return TWideString();
    }

    CDependencyTree* BuildDependencyTree() const
    {
        // The only dependencies STRGs have is they can reference FONTs with the &font=; formatting tag
        CDependencyTree *pTree = new CDependencyTree(ID());
        EIDLength IDLength = (Game() <= eEchoes ? e32Bit : e64Bit);

        for (u32 iLang = 0; iLang < mLangTables.size(); iLang++)
        {
            const SLangTable& rkTable = mLangTables[iLang];

            for (u32 iStr = 0; iStr < rkTable.Strings.size(); iStr++)
            {
                static const TWideString skTag = L"&font=";
                const TWideString& rkStr = rkTable.Strings[iStr];

                for (u32 FontIdx = rkStr.IndexOfPhrase(*skTag); FontIdx != -1; FontIdx = rkStr.IndexOfPhrase(*skTag, FontIdx + 1))
                {
                    u32 IDStart = FontIdx + skTag.Size();
                    TWideString StrFontID = rkStr.SubString(IDStart, IDLength * 2);
                    pTree->AddDependency( CAssetID::FromString(StrFontID) );
                }
            }
        }

        return pTree;
    }
};

#endif // CSTRINGTABLE_H
