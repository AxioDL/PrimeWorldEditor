#ifndef CWORLDCOOKER_H
#define CWORLDCOOKER_H

#include <Common/types.h>
#include "../EFormatVersion.h"

class CWorldCooker
{
    CWorldCooker();
public:
    static u32 GetMLVLVersion(EGame version);
};

#endif // CWORLDCOOKER_H
