#include "CWorld.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Script/CScriptLayer.h"

CWorld::CWorld(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CWorld::~CWorld() = default;

std::unique_ptr<CDependencyTree> CWorld::BuildDependencyTree() const
{
    auto pTree = std::make_unique<CDependencyTree>();

    for (const auto& area : mAreas)
    {
        pTree->AddDependency(area.AreaResID);
        pTree->AddDependency(area.pAreaName);
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
    for (uint32 iArea = 0; iArea < mAreas.size(); iArea++)
    {
        if (mAreas[iArea].AreaResID == pArea->ID())
        {
            pArea->SetWorldIndex(iArea);
            break;
        }
    }

    SArea& AreaInfo = mAreas[pArea->WorldIndex()];

    for (size_t iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
    {
        if (AreaInfo.Layers.size() <= iLyr)
            break;

        CScriptLayer *pLayer = pArea->ScriptLayer(iLyr);
        SArea::SLayer& rLayerInfo = AreaInfo.Layers[iLyr];

        pLayer->SetName(rLayerInfo.LayerName);
        pLayer->SetActive(rLayerInfo.Active);
    }
}

TString CWorld::InGameName() const
{
    if (mpWorldName)
        return mpWorldName->GetString(ELanguage::English, 0);
    else
        return Entry()->Name();
}

TString CWorld::AreaInGameName(uint32 AreaIndex) const
{
    const SArea& rkArea = mAreas[AreaIndex];

    if (rkArea.pAreaName)
        return rkArea.pAreaName->GetString(ELanguage::English, 0);
    else
        return "!!" + rkArea.InternalName;
}

uint32 CWorld::AreaIndex(CAssetID AreaID) const
{
    for (uint32 AreaIdx = 0; AreaIdx < mAreas.size(); AreaIdx++)
    {
        if (mAreas[AreaIdx].AreaResID == AreaID)
            return AreaIdx;
    }

    return UINT32_MAX;
}

// ************ SERIALIZATION ************
void CWorld::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("Name", mName)
         << SerialParameter("NameString", mpWorldName);

    if (rArc.Game() == EGame::EchoesDemo || rArc.Game() == EGame::Echoes)
        rArc << SerialParameter("DarkNameString", mpDarkWorldName);

    rArc << SerialParameter("WorldSaveInfo", mpSaveWorld)
         << SerialParameter("WorldMap", mpMapWorld)
         << SerialParameter("DefaultSkyModel", mpDefaultSkybox);

    if (rArc.Game() >= EGame::EchoesDemo && rArc.Game() <= EGame::Corruption)
        rArc << SerialParameter("TempleKeyWorldIndex", mTempleKeyWorldIndex);

    if (rArc.Game() == EGame::DKCReturns)
        rArc << SerialParameter("TimeAttackData", mTimeAttackData);

    if (rArc.Game() == EGame::Prime)
        rArc << SerialParameter("MemoryRelays", mMemoryRelays);

    rArc << SerialParameter("Areas", mAreas);
}

void Serialize(IArchive& rArc, CWorld::STimeAttackData& rData)
{
    rArc << SerialParameter("HasTimeAttack", rData.HasTimeAttack)
         << SerialParameter("ActNumber", rData.ActNumber)
         << SerialParameter("BronzeTime", rData.BronzeTime)
         << SerialParameter("SilverTime", rData.SilverTime)
         << SerialParameter("GoldTime", rData.GoldTime)
         << SerialParameter("ShinyGoldTime", rData.ShinyGoldTime);
}

void Serialize(IArchive& rArc, CWorld::SMemoryRelay& rMemRelay)
{
    rArc << SerialParameter("MemoryRelayID", rMemRelay.InstanceID, SH_HexDisplay)
         << SerialParameter("TargetID", rMemRelay.TargetID, SH_HexDisplay)
         << SerialParameter("Message", rMemRelay.Message)
         << SerialParameter("Active", rMemRelay.Active);
}

void Serialize(IArchive& rArc, CWorld::SArea& rArea)
{
    rArc << SerialParameter("Name", rArea.InternalName)
         << SerialParameter("NameString", rArea.pAreaName)
         << SerialParameter("MREA", rArea.AreaResID)
         << SerialParameter("ID", rArea.AreaID)
         << SerialParameter("Transform", rArea.Transform)
         << SerialParameter("BoundingBox", rArea.AetherBox)
         << SerialParameter("AllowPakDuplicates", rArea.AllowPakDuplicates)
         << SerialParameter("AttachedAreas", rArea.AttachedAreaIDs)
         << SerialParameter("RelModules", rArea.RelFilenames)
         << SerialParameter("RelOffsets", rArea.RelOffsets)
         << SerialParameter("Docks", rArea.Docks)
         << SerialParameter("Layers", rArea.Layers);
}

void Serialize(IArchive& rArc, CWorld::SArea::SDock& rDock)
{
    rArc << SerialParameter("ConnectingDocks", rDock.ConnectingDocks)
         << SerialParameter("DockCoords", rDock.DockCoordinates);
}

void Serialize(IArchive& rArc, CWorld::SArea::SDock::SConnectingDock& rDock)
{
    rArc << SerialParameter("AreaIndex", rDock.AreaIndex)
         << SerialParameter("DockIndex", rDock.DockIndex);
}

void Serialize(IArchive& rArc, CWorld::SArea::SLayer& rLayer)
{
    rArc << SerialParameter("Name", rLayer.LayerName)
         << SerialParameter("Active", rLayer.Active);

    if (rArc.Game() >= EGame::Corruption)
    {
        rArc << SerialParameter("StateID", rLayer.LayerStateID);
    }
}

void Serialize(IArchive& rArc, CWorld::SAudioGrp& rAudioGrp)
{
    rArc << SerialParameter("GroupID", rAudioGrp.GroupID)
         << SerialParameter("AGSC", rAudioGrp.ResID);
}
