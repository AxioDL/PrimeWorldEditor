#ifndef CTWEAKLOADER_H
#define CTWEAKLOADER_H

#include "CTweakData.h"

/** Class responsible for loading tweak data */
class CTweakLoader
{
    /** Private constructor */
    CTweakLoader() {}

public:
    /** Loader entry point */
    static CTweakData* LoadCTWK(IInputStream& CTWK, CResourceEntry* pEntry);
    static void LoadNTWK(IInputStream& NTWK, EGame Game, std::vector<CTweakData*>& OutTweaks);
};

#endif // CTWEAKLOADER_H
