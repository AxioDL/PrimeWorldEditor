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
    void LoadPrimeDemoSTRG(IInputStream& rSTRG);
    void LoadPrimeSTRG(IInputStream& rSTRG);
    void LoadCorruptionSTRG(IInputStream& rSTRG);
    void LoadNameTable(IInputStream& rSTRG);

public:
    static CStringTable* LoadSTRG(IInputStream& rSTRG);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CSTRINGLOADER_H
