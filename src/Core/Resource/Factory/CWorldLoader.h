#ifndef CWORLDLOADER_H
#define CWORLDLOADER_H

#include "Core/Resource/CWorld.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EGame.h"

#include <FileIO/FileIO.h>

class CWorldLoader
{
    TResPtr<CWorld> mpWorld;
    EGame mVersion;

    CWorldLoader();
    void LoadPrimeMLVL(IInputStream& rMLVL);
    void LoadReturnsMLVL(IInputStream& rMLVL);

public:
    static CWorld* LoadMLVL(IInputStream& rMLVL);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CWORLDLOADER_H
