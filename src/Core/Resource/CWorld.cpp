#include "CWorld.h"
#include "CResCache.h"
#include "Core/Resource/Script/CScriptLayer.h"

CWorld::CWorld() : CResource()
{
    mWorldVersion = eUnknownVersion;
    mpWorldName = nullptr;
    mpDarkWorldName = nullptr;
    mpSaveWorld = nullptr;
    mpDefaultSkybox = nullptr;
    mpMapWorld = nullptr;
}

CWorld::~CWorld()
{
}

void CWorld::SetAreaLayerInfo(CGameArea *pArea, u32 AreaIndex)
{
    // The AreaIndex parameter is a placeholder until an improved world loader is implemented.
    // For now it's the easiest/fastest way to do this because this function is called from
    // the start window and the start window already knows the area index.
    SArea& AreaInfo = mAreas[AreaIndex];

    for (u32 iLyr = 0; iLyr < pArea->GetScriptLayerCount(); iLyr++)
    {
        CScriptLayer *pLayer = pArea->GetScriptLayer(iLyr);
        SArea::SLayer& LayerInfo = AreaInfo.Layers[iLyr];

        pLayer->SetName(LayerInfo.LayerName);
        pLayer->SetActive(LayerInfo.EnabledByDefault);
    }
}

// ************ GETTERS ************
// World
EGame CWorld::Version()
{
    return mWorldVersion;
}

CStringTable* CWorld::GetWorldName()
{
    return mpWorldName;
}

CStringTable* CWorld::GetDarkWorldName()
{
    return mpDarkWorldName;
}

CResource* CWorld::GetSaveWorld()
{
    return mpSaveWorld;
}

CModel* CWorld::GetDefaultSkybox()
{
    return mpDefaultSkybox;
}

CResource* CWorld::GetMapWorld()
{
    return mpMapWorld;
}

// Area
u32 CWorld::GetNumAreas()
{
    return mAreas.size();
}

u64 CWorld::GetAreaResourceID(u32 AreaIndex)
{
    return mAreas[AreaIndex].FileID;
}

u32 CWorld::GetAreaAttachedCount(u32 AreaIndex)
{
    return mAreas[AreaIndex].AttachedAreaIDs.size();
}

u32 CWorld::GetAreaAttachedID(u32 AreaIndex, u32 AttachedIndex)
{
    return (u32) mAreas[AreaIndex].AttachedAreaIDs[AttachedIndex];
}

TString CWorld::GetAreaInternalName(u32 AreaIndex)
{
    return mAreas[AreaIndex].InternalName;
}

CStringTable* CWorld::GetAreaName(u32 AreaIndex)
{
    return mAreas[AreaIndex].pAreaName;
}
