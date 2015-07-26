#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "../CStringTable.h"
#include "../EFormatVersion.h"
#include <Core/CResCache.h>

class CStringLoader
{
    CStringTable* mpStringTable;
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
