#ifndef CWORLDLOADER_H
#define CWORLDLOADER_H

#include "Core/Resource/CWorld.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EFormatVersion.h"

#include <FileIO/FileIO.h>

class CWorldLoader
{
    TResPtr<CWorld> mpWorld;
    EGame mVersion;

    CWorldLoader();
    void LoadPrimeMLVL(CInputStream& MLVL);
    void LoadReturnsMLVL(CInputStream& MLVL);

public:
    static CWorld* LoadMLVL(CInputStream& MLVL);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CWORLDLOADER_H
