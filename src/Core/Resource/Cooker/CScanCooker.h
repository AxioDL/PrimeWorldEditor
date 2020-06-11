#ifndef CSCANCOOKER_H
#define CSCANCOOKER_H

#include "Core/Resource/Scan/CScan.h"

/** Cooker class for writing game-compatible SCAN assets */
class CScanCooker
{
    CScanCooker() = default;

public:
    static bool CookSCAN(CScan* pScan, IOutputStream& SCAN);
};

#endif // CSCANCOOKER_H
