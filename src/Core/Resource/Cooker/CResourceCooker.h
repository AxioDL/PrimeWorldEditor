#ifndef CRESOURCECOOKER_H
#define CRESOURCECOOKER_H

#include "CAreaCooker.h"
#include "CModelCooker.h"
#include "CPoiToWorldCooker.h"
#include "CScanCooker.h"
#include "CStringCooker.h"
#include "CWorldCooker.h"

#include "Core/Tweaks/CTweakCooker.h"

#include "Core/GameProject/CResourceEntry.h"

class CResourceCooker
{
    CResourceCooker() = default;

public:
    static bool CookResource(CResourceEntry *pEntry, IOutputStream& rOutput)
    {
        CResource *pRes = pEntry->Load();
        ASSERT(pRes != nullptr);

        switch (pEntry->ResourceType())
        {
        case EResourceType::Area:                 return CAreaCooker::CookMREA((CGameArea*) pRes, rOutput);
        case EResourceType::Model:                return CModelCooker::CookCMDL((CModel*) pRes, rOutput);
        case EResourceType::Scan:                 return CScanCooker::CookSCAN((CScan*) pRes, rOutput);
        case EResourceType::StaticGeometryMap:    return CPoiToWorldCooker::CookEGMC((CPoiToWorld*) pRes, rOutput);
        case EResourceType::StringTable:          return CStringCooker::CookSTRG((CStringTable*) pRes, rOutput);
        case EResourceType::Tweaks:               return CTweakCooker::CookCTWK((CTweakData*) pRes, rOutput);
        case EResourceType::World:                return CWorldCooker::CookMLVL((CWorld*) pRes, rOutput);

        default:
            warnf("Failed to cook %s asset; this resource type is not supported for cooking", *pEntry->CookedExtension().ToString());
            return false;
        }
    }
};

#endif // CRESOURCECOOKER_H
