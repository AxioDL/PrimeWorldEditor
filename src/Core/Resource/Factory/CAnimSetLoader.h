#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/CAnimSet.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/CResCache.h"

class CAnimSetLoader
{
    TResPtr<CAnimSet> pSet;
    CResCache *mpResCache;
    EGame mVersion;

    CAnimSetLoader();
    CAnimSet* LoadCorruptionCHAR(IInputStream& rCHAR);
    CAnimSet* LoadReturnsCHAR(IInputStream& rCHAR);
    void LoadPASDatabase(IInputStream& rPAS4);

public:
    static CAnimSet* LoadANCS(IInputStream& rANCS);
    static CAnimSet* LoadCHAR(IInputStream& rCHAR);
};

#endif // CCHARACTERLOADER_H
