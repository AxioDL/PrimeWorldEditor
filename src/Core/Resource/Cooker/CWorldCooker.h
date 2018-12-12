#ifndef CWORLDCOOKER_H
#define CWORLDCOOKER_H

#include "Core/Resource/CWorld.h"
#include <Common/BasicTypes.h>
#include <Common/EGame.h>

class CWorldCooker
{
    CWorldCooker();
public:
    static bool CookMLVL(CWorld *pWorld, IOutputStream& rOut);
    static uint32 GetMLVLVersion(EGame Version);
};

#endif // CWORLDCOOKER_H
