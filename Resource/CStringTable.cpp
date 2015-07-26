#include "CStringTable.h"

CStringTable::CStringTable() : CResource()
{
}

CStringTable::~CStringTable()
{
}

EResType CStringTable::Type()
{
    return eStringTable;
}

CResource* CStringTable::MakeCopy(CResCache*)
{
    // Not using parameter 1 (CResCache* - pResCache)
    return new CStringTable(*this);
}

// ************ SETTERS ************
u32 CStringTable::GetStringCount()
{
    return mNumStrings;
}

u32 CStringTable::GetLangCount()
{
    return mLangTables.size();
}

CFourCC CStringTable::GetLangTag(u32 Index)
{
    return mLangTables[Index].Language;
}

std::wstring CStringTable::GetString(CFourCC Lang, u32 StringIndex)
{
    for (u32 iLang = 0; iLang < GetLangCount(); iLang++)
    {
        if (GetLangTag(iLang) == Lang)
            return GetString(iLang, StringIndex);
    }

    return std::wstring();
}

std::wstring CStringTable::GetString(u32 LangIndex, u32 StringIndex)
{
    return mLangTables[LangIndex].Strings[StringIndex];
}

std::string CStringTable::GetStringName(u32 StringIndex)
{
    return mStringNames[StringIndex];
}
