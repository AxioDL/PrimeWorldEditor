#ifndef CWORLDCOOKER_H
#define CWORLDCOOKER_H

#include "Core/Resource/EFormatVersion.h"
#include <Common/types.h>

class CWorldCooker
{
    CWorldCooker();
public:
    static u32 GetMLVLVersion(EGame version);
};

#endif // CWORLDCOOKER_H
