#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/StringTable/CStringTable.h"
#include <Common/EGame.h>

class CStringLoader
{
    CStringTable *mpStringTable;
    EGame mVersion;

    CStringLoader() {}
    void LoadPrimeDemoSTRG(IInputStream& STRG);
    void LoadPrimeSTRG(IInputStream& STRG);
    void LoadCorruptionSTRG(IInputStream& STRG);
    void LoadNameTable(IInputStream& STRG);

public:
    static CStringTable* LoadSTRG(IInputStream& STRG, CResourceEntry* pEntry);
    static EGame GetFormatVersion(uint Version);
};

#endif // CSTRINGLOADER_H
