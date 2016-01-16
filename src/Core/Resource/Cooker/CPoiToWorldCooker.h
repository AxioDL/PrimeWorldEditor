#ifndef CPOITOWORLDCOOKER_H
#define CPOITOWORLDCOOKER_H

#include "Core/Resource/CPoiToWorld.h"
#include <FileIO/FileIO.h>

class CPoiToWorldCooker
{
    CPoiToWorldCooker() {}
public:
    static void WriteEGMC(CPoiToWorld *pPoiToWorld, IOutputStream& rOut);
};

#endif // CPOITOWORLDCOOKER_H
