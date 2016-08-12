#ifndef CWORLDCOOKER_H
#define CWORLDCOOKER_H

#include "Core/Resource/CWorld.h"
#include "Core/Resource/EGame.h"
#include <Common/types.h>

class CWorldCooker
{
    CWorldCooker();
public:
    static bool CookMLVL(CWorld *pWorld, IOutputStream& rOut);
    static u32 GetMLVLVersion(EGame Version);
};

#endif // CWORLDCOOKER_H
