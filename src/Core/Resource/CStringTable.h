#ifndef CSTRINGTABLE_H
#define CSTRINGTABLE_H

#include "CResource.h"
#include <Common/types.h>
#include <Common/CFourCC.h>
#include <vector>
#include <string>

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
    CStringTable();
    ~CStringTable();
    CResource* MakeCopy(CResCache *pCopyCache);

    // Getters
    u32 GetStringCount();
    u32 GetLangCount();
    CFourCC GetLangTag(u32 Index);
    TWideString GetString(CFourCC Lang, u32 StringIndex);
    TWideString GetString(u32 LangIndex, u32 StringIndex);
    TString GetStringName(u32 StringIndex);
};

#endif // CSTRINGTABLE_H
