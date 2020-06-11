#ifndef CPOITOWORLDCOOKER_H
#define CPOITOWORLDCOOKER_H

#include "Core/Resource/CPoiToWorld.h"
#include <Common/FileIO.h>

class CPoiToWorldCooker
{
    CPoiToWorldCooker() = default;

public:
    static bool CookEGMC(CPoiToWorld *pPoiToWorld, IOutputStream& rOut);
};

#endif // CPOITOWORLDCOOKER_H
