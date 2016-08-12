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
    CDependencyTree *pTree = new CDependencyTree(ID());

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

    if (Game() <= ePrime)
    {
        for (u32 iGrp = 0; iGrp < mAudioGrps.size(); iGrp++)
            pTree->AddDependency(mAudioGrps[iGrp].ResID);
    }

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

// ************ SERIALIZATION ************
void CWorld::Serialize(IArchive& rArc)
{
    rArc << SERIAL("WorldNameSTRG", mpWorldName)
         << SERIAL("DarkWorldNameSTRG", mpDarkWorldName)
         << SERIAL("WorldSaveInfo", mpSaveWorld)
         << SERIAL("DefaultSkyCMDL", mpDefaultSkybox)
         << SERIAL("MapWorld", mpMapWorld)
         << SERIAL("Unknown1", mUnknown1)
         << SERIAL("UnknownAreas", mUnknownAreas)
         << SERIAL("UnknownAGSC", mUnknownAGSC)
         << SERIAL_CONTAINER("MemoryRelays", mMemoryRelays, "MemoryRelay")
         << SERIAL_CONTAINER("Areas", mAreas, "Area")
         << SERIAL_CONTAINER("AudioGroups", mAudioGrps, "AudioGroup");
}

void Serialize(IArchive& rArc, CWorld::SMemoryRelay& rMemRelay)
{
    rArc << SERIAL_HEX("MemoryRelayID", rMemRelay.InstanceID)
         << SERIAL_HEX("TargetID", rMemRelay.TargetID)
         << SERIAL("Message", rMemRelay.Message)
         << SERIAL("Unknown", rMemRelay.Unknown);
}

void Serialize(IArchive& rArc, CWorld::SArea& rArea)
{
    rArc << SERIAL("Name", rArea.InternalName)
         << SERIAL("NameSTRG", rArea.pAreaName)
         << SERIAL("Transform", rArea.Transform)
         << SERIAL("BoundingBox", rArea.AetherBox)
         << SERIAL("AreaMREA", rArea.AreaResID)
         << SERIAL_HEX("AreaID", rArea.AreaID)
         << SERIAL("AllowPakDuplicates", rArea.AllowPakDuplicates)
         << SERIAL_CONTAINER("AttachedAreas", rArea.AttachedAreaIDs, "AreaIndex")
         << SERIAL_CONTAINER("Dependencies", rArea.Dependencies, "Dependency")
         << SERIAL_CONTAINER("RelModules", rArea.RelFilenames, "Module")
         << SERIAL_CONTAINER("RelOffsets", rArea.RelOffsets, "Offset")
         << SERIAL("CommonDependsStart", rArea.CommonDependenciesStart)
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
         << SERIAL("DefaultEnabled", rLayer.EnabledByDefault)
         << SERIAL("LayerDependsStart", rLayer.LayerDependenciesStart);
}

void Serialize(IArchive& rArc, CWorld::SAudioGrp& rAudioGrp)
{
    rArc << SERIAL("StudioID", rAudioGrp.Unknown)
         << SERIAL("AGSC", rAudioGrp.ResID);
}
