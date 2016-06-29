#ifndef CPOITOWORLDLOADER_H
#define CPOITOWORLDLOADER_H

#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/TResPtr.h"

class CPoiToWorldLoader
{
    TResPtr<CPoiToWorld> mpPoiToWorld;

    CPoiToWorldLoader() {}

public:
    static CPoiToWorld* LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry);
};

#endif // CPOITOWORLDLOADER_H
