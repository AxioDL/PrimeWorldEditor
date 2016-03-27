#include "CWorld.h"
#include "CResCache.h"
#include "Core/Resource/Script/CScriptLayer.h"

CWorld::CWorld()
    : CResource()
    , mWorldVersion(eUnknownVersion)
    , mpWorldName(nullptr)
    , mpDarkWorldName(nullptr)
    , mpSaveWorld(nullptr)
    , mpDefaultSkybox(nullptr)
    , mpMapWorld(nullptr)
{
}

CWorld::~CWorld()
{
}

void CWorld::SetAreaLayerInfo(CGameArea *pArea)
{
    // The AreaIndex parameter is a placeholder until an improved world loader is implemented.
    // For now it's the easiest/fastest way to do this because this function is called from
    // the start window and the start window already knows the area index.
    SArea& AreaInfo = mAreas[pArea->WorldIndex()];

    for (u32 iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
    {
        if (AreaInfo.Layers.size() <= iLyr) break;
        CScriptLayer *pLayer = pArea->ScriptLayer(iLyr);
        SArea::SLayer& rLayerInfo = AreaInfo.Layers[iLyr];

        pLayer->SetName(rLayerInfo.LayerName);
        pLayer->SetActive(rLayerInfo.EnabledByDefault);
    }
}
