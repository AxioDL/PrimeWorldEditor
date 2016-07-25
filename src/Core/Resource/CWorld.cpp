#include "CWorld.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Script/CScriptLayer.h"

CWorld::CWorld(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
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

CDependencyTree* CWorld::BuildDependencyTree() const
{
    CDependencyTree *pTree = new CDependencyTree(ResID());

    for (u32 iArea = 0; iArea < mAreas.size(); iArea++)
    {
        pTree->AddDependency(mAreas[iArea].FileID);
        pTree->AddDependency(mAreas[iArea].pAreaName);
    }
    
    pTree->AddDependency(mpWorldName);
    pTree->AddDependency(mpDarkWorldName);
    pTree->AddDependency(mpSaveWorld);
    pTree->AddDependency(mpDefaultSkybox);
    pTree->AddDependency(mpMapWorld);

    if (Game() <= ePrime)
        Log::Warning("CWorld::BuildDependencyTree not handling audio groups");

    return pTree;
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
