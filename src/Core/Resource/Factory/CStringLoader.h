#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "Core/Resource/CStringTable.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/TResPtr.h"

class CStringLoader
{
    TResPtr<CStringTable> mpStringTable;
    EGame mVersion;

    CStringLoader();
    void LoadPrimeDemoSTRG(IInputStream& STRG);
    void LoadPrimeSTRG(IInputStream& STRG);
    void LoadCorruptionSTRG(IInputStream& STRG);
    void LoadNameTable(IInputStream& STRG);

public:
    static CStringTable* LoadSTRG(IInputStream& STRG);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CSTRINGLOADER_H
