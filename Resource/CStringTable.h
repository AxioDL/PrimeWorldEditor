#ifndef CSTRINGTABLE_H
#define CSTRINGTABLE_H

#include "CResource.h"
#include <Common/types.h>
#include <Common/CFourCC.h>
#include <vector>
#include <string>

class CStringTable : public CResource
{
    friend class CStringLoader;

    std::vector<std::string> mStringNames;
    u32 mNumStrings;

    struct SLangTable
    {
        CFourCC Language;
        std::vector<std::wstring> Strings;
    };
    std::vector<SLangTable> mLangTables;

public:
    CStringTable();
    ~CStringTable();
    EResType Type();
    CResource* MakeCopy(CResCache *pCopyCache);

    // Getters
    u32 GetStringCount();
    u32 GetLangCount();
    CFourCC GetLangTag(u32 Index);
    std::wstring GetString(CFourCC Lang, u32 StringIndex);
    std::wstring GetString(u32 LangIndex, u32 StringIndex);
    std::string GetStringName(u32 StringIndex);
};

#endif // CSTRINGTABLE_H
