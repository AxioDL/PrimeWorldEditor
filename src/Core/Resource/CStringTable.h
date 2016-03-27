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
    CStringTable() {}

    inline u32 NumStrings() const               { return mLangTables.size(); }
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
};

#endif // CSTRINGTABLE_H
