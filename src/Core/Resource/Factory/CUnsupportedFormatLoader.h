#ifndef CUNSUPPORTEDFORMATLOADER_H
#define CUNSUPPORTEDFORMATLOADER_H

#include "Core/Resource/CAudioMacro.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CMapArea.h"

// This class is responsible for loading formats that aren't yet fully supported.
// This is needed so we have access to the full dependency list of all resource types.
class CUnsupportedFormatLoader
{
    CDependencyGroup *mpGroup = nullptr;
    CUnsupportedFormatLoader() = default;

    static void PerformCheating(IInputStream& rFile, EGame Game, std::list<CAssetID>& rAssetList);

public:
    static CAudioMacro*      LoadCAUD(IInputStream& rCAUD, CResourceEntry *pEntry);
    static CDependencyGroup* LoadCSNG(IInputStream& rCSNG, CResourceEntry *pEntry);
    static CDependencyGroup* LoadDUMB(IInputStream& rDUMB, CResourceEntry *pEntry);
    static CDependencyGroup* LoadFRME(IInputStream& rFRME, CResourceEntry *pEntry);
    static CDependencyGroup* LoadFSM2(IInputStream& rFSM2, CResourceEntry *pEntry);
    static CDependencyGroup* LoadFSMC(IInputStream& rFSMC, CResourceEntry *pEntry);
    static CDependencyGroup* LoadHIER(IInputStream& rHIER, CResourceEntry *pEntry);
    static CDependencyGroup* LoadHINT(IInputStream& rHINT, CResourceEntry *pEntry);
    static CMapArea*         LoadMAPA(IInputStream& rMAPA, CResourceEntry *pEntry);
    static CDependencyGroup* LoadMAPW(IInputStream& rMAPW, CResourceEntry *pEntry);
    static CDependencyGroup* LoadMAPU(IInputStream& rMAPU, CResourceEntry *pEntry);
    static CDependencyGroup* LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry);
};

#endif // CUNSUPPORTEDFORMATLOADER_H
