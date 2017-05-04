#include "CWorld.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Script/CScriptLayer.h"

CWorld::CWorld(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mpWorldName(nullptr)
    , mpDarkWorldName(nullptr)
    , mpSaveWorld(nullptr)
    , mpDefaultSkybox(nullptr)
    , mpMapWorld(nullptr)
    , mTempleKeyWorldIndex(0)
{
}

CWorld::~CWorld()
{
}

CDependencyTree* CWorld::BuildDependencyTree() const
{
    CDependencyTree *pTree = new CDependencyTree();

    for (u32 iArea = 0; iArea < mAreas.size(); iArea++)
    {
        pTree->AddDependency(mAreas[iArea].AreaResID);
        pTree->AddDependency(mAreas[iArea].pAreaName);
    }
    
    pTree->AddDependency(mpWorldName);
    pTree->AddDependency(mpDarkWorldName);
    pTree->AddDependency(mpSaveWorld);
    pTree->AddDependency(mpDefaultSkybox);
    pTree->AddDependency(mpMapWorld);

    return pTree;
}

void CWorld::SetAreaLayerInfo(CGameArea *pArea)
{
    for (u32 iArea = 0; iArea < mAreas.size(); iArea++)
    {
        if (mAreas[iArea].AreaResID == pArea->ID())
        {
            pArea->SetWorldIndex(iArea);
            break;
        }
    }

    SArea& AreaInfo = mAreas[pArea->WorldIndex()];

    for (u32 iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
    {
        if (AreaInfo.Layers.size() <= iLyr) break;
        CScriptLayer *pLayer = pArea->ScriptLayer(iLyr);
        SArea::SLayer& rLayerInfo = AreaInfo.Layers[iLyr];

        pLayer->SetName(rLayerInfo.LayerName);
        pLayer->SetActive(rLayerInfo.Active);
    }
}

TString CWorld::InGameName() const
{
    if (mpWorldName)
        return mpWorldName->String("ENGL", 0).ToUTF8();
    else
        return Entry()->Name();
}

TString CWorld::AreaInGameName(u32 AreaIndex) const
{
    const SArea& rkArea = mAreas[AreaIndex];

    if (rkArea.pAreaName)
        return rkArea.pAreaName->String("ENGL", 0).ToUTF8();
    else
        return "!!" + rkArea.InternalName;
}

// ************ SERIALIZATION ************
void CWorld::Serialize(IArchive& rArc)
{
    rArc << SERIAL("Name", mName)
         << SERIAL("NameString", mpWorldName);

    if (rArc.Game() == eEchoesDemo || rArc.Game() == eEchoes)
        rArc << SERIAL("DarkNameString", mpDarkWorldName);

    rArc << SERIAL("WorldSaveInfo", mpSaveWorld)
         << SERIAL("WorldMap", mpMapWorld)
         << SERIAL("DefaultSkyModel", mpDefaultSkybox);

    if (rArc.Game() >= eEchoesDemo && rArc.Game() <= eCorruption)
        rArc << SERIAL("TempleKeyWorldIndex", mTempleKeyWorldIndex);

    if (rArc.Game() == ePrime)
        rArc << SERIAL_CONTAINER("MemoryRelays", mMemoryRelays, "MemoryRelay");

    rArc << SERIAL_CONTAINER("Areas", mAreas, "Area");
}

void Serialize(IArchive& rArc, CWorld::SMemoryRelay& rMemRelay)
{
    rArc << SERIAL_HEX("MemoryRelayID", rMemRelay.InstanceID)
         << SERIAL_HEX("TargetID", rMemRelay.TargetID)
         << SERIAL("Message", rMemRelay.Message)
         << SERIAL("Active", rMemRelay.Active);
}

void Serialize(IArchive& rArc, CWorld::SArea& rArea)
{
    rArc << SERIAL("Name", rArea.InternalName)
         << SERIAL("NameString", rArea.pAreaName)
         << SERIAL("MREA", rArea.AreaResID)
         << SERIAL("ID", rArea.AreaID)
         << SERIAL("Transform", rArea.Transform)
         << SERIAL("BoundingBox", rArea.AetherBox)
         << SERIAL("AllowPakDuplicates", rArea.AllowPakDuplicates)
         << SERIAL_CONTAINER("AttachedAreas", rArea.AttachedAreaIDs, "AreaIndex")
         << SERIAL_CONTAINER("RelModules", rArea.RelFilenames, "Module")
         << SERIAL_CONTAINER("RelOffsets", rArea.RelOffsets, "Offset")
         << SERIAL_CONTAINER("Docks", rArea.Docks, "Dock")
         << SERIAL_CONTAINER("Layers", rArea.Layers, "Layer");
}

void Serialize(IArchive& rArc, CWorld::SArea::SDock& rDock)
{
    rArc << SERIAL_CONTAINER("ConnectingDocks", rDock.ConnectingDocks, "ConnectingDock")
         << SERIAL_CONTAINER("DockCoords", rDock.DockCoordinates, "Coord");
}

void Serialize(IArchive& rArc, CWorld::SArea::SDock::SConnectingDock& rDock)
{
    rArc << SERIAL("AreaIndex", rDock.AreaIndex)
         << SERIAL("DockIndex", rDock.DockIndex);
}

void Serialize(IArchive& rArc, CWorld::SArea::SLayer& rLayer)
{
    rArc << SERIAL("Name", rLayer.LayerName)
         << SERIAL("Active", rLayer.Active);
}

void Serialize(IArchive& rArc, CWorld::SAudioGrp& rAudioGrp)
{
    rArc << SERIAL("GroupID", rAudioGrp.GroupID)
         << SERIAL("AGSC", rAudioGrp.ResID);
}
