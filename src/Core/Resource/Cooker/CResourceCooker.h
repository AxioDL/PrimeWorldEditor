#ifndef CRESOURCECOOKER_H
#define CRESOURCECOOKER_H

#include "CAreaCooker.h"
#include "CModelCooker.h"
#include "CPoiToWorldCooker.h"
#include "CWorldCooker.h"

#include "Core/GameProject/CResourceEntry.h"

class CResourceCooker
{
    CResourceCooker() {}

public:
    static bool CookResource(CResourceEntry *pEntry, IOutputStream& rOutput)
    {
        CResource *pRes = pEntry->Load();
        ASSERT(pRes != nullptr);

        switch (pEntry->ResourceType())
        {
        case eArea:                 return CAreaCooker::CookMREA((CGameArea*) pRes, rOutput);
        case eModel:                return CModelCooker::CookCMDL((CModel*) pRes, rOutput);
        case eStaticGeometryMap:    return CPoiToWorldCooker::CookEGMC((CPoiToWorld*) pRes, rOutput);
        case eWorld:                return CWorldCooker::CookMLVL((CWorld*) pRes, rOutput);

        default:
            warnf("Failed to cook %s asset; this resource type is not supported for cooking", *pEntry->CookedExtension().ToString());
            return false;
        }
    }
};

#endif // CRESOURCECOOKER_H
