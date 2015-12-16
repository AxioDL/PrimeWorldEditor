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
    void LoadPrimeMLVL(IInputStream& MLVL);
    void LoadReturnsMLVL(IInputStream& MLVL);

public:
    static CWorld* LoadMLVL(IInputStream& MLVL);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CWORLDLOADER_H
