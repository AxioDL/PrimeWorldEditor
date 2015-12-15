#ifndef CCHARACTERLOADER_H
#define CCHARACTERLOADER_H

#include "Core/Resource/CAnimSet.h"
#include "Core/Resource/EFormatVersion.h"
#include "Core/Resource/CResCache.h"

class CAnimSetLoader
{
    TResPtr<CAnimSet> set;
    CResCache *mpResCache;
    EGame mVersion;

    CAnimSetLoader();
    CAnimSet* LoadCorruptionCHAR(CInputStream& CHAR);
    CAnimSet* LoadReturnsCHAR(CInputStream& CHAR);
    void LoadPASDatabase(CInputStream& PAS4);

public:
    static CAnimSet* LoadANCS(CInputStream& ANCS);
    static CAnimSet* LoadCHAR(CInputStream& CHAR);
};

#endif // CCHARACTERLOADER_H
