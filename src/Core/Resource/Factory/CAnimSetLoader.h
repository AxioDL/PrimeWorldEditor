#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/CAnimSet.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/CResCache.h"

class CAnimSetLoader
{
    TResPtr<CAnimSet> set;
    CResCache *mpResCache;
    EGame mVersion;

    CAnimSetLoader();
    CAnimSet* LoadCorruptionCHAR(IInputStream& CHAR);
    CAnimSet* LoadReturnsCHAR(IInputStream& CHAR);
    void LoadPASDatabase(IInputStream& PAS4);

public:
    static CAnimSet* LoadANCS(IInputStream& ANCS);
    static CAnimSet* LoadCHAR(IInputStream& CHAR);
};

#endif // CCHARACTERLOADER_H
