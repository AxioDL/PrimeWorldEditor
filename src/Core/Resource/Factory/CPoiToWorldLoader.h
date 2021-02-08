#ifndef CPOITOWORLDLOADER_H
#define CPOITOWORLDLOADER_H

#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/TResPtr.h"
#include <memory>

class CPoiToWorldLoader
{
    CPoiToWorldLoader() = default;

public:
    static std::unique_ptr<CPoiToWorld> LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry);
};

#endif // CPOITOWORLDLOADER_H
