#ifndef CTWEAKCOOKER_H
#define CTWEAKCOOKER_H

#include "CTweakData.h"

/** Class responsible for cooking tweak data */
class CTweakCooker
{
    /** Private constructor */
    CTweakCooker() = default;

public:
    /** Cooker entry point */
    static bool CookCTWK(CTweakData* pTweakData, IOutputStream& CTWK);
    static bool CookNTWK(const std::vector<CTweakData*>& kTweaks, IOutputStream& NTWK);
};

#endif // CTWEAKCOOKER_H
