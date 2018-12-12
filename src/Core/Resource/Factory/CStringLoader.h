#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CStringTable.h"
#include "Core/Resource/TResPtr.h"
#include <Common/EGame.h>

class CStringLoader
{
    CStringTable *mpStringTable;
    EGame mVersion;

    CStringLoader() {}
    void LoadPrimeDemoSTRG(IInputStream& rSTRG);
    void LoadPrimeSTRG(IInputStream& rSTRG);
    void LoadCorruptionSTRG(IInputStream& rSTRG);
    void LoadNameTable(IInputStream& rSTRG);

public:
    static CStringTable* LoadSTRG(IInputStream &rSTRG, CResourceEntry *pEntry);
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CSTRINGLOADER_H
