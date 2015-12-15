#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "Core/Resource/CStringTable.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EFormatVersion.h"
#include "Core/Resource/TResPtr.h"

class CStringLoader
{
    TResPtr<CStringTable> mpStringTable;
    EGame mVersion;

    CStringLoader();
    void LoadPrimeDemoSTRG(CInputStream& STRG);
    void LoadPrimeSTRG(CInputStream& STRG);
    void LoadCorruptionSTRG(CInputStream& STRG);
    void LoadNameTable(CInputStream& STRG);

public:
    static CStringTable* LoadSTRG(CInputStream& STRG);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CSTRINGLOADER_H
