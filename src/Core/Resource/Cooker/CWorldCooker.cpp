#include "CWorldCooker.h"
#include "Core/GameProject/DependencyListBuilders.h"

CWorldCooker::CWorldCooker()
{
}

// ************ STATIC ************
bool CWorldCooker::CookMLVL(CWorld *pWorld, IOutputStream& rMLVL)
{
    ASSERT(rMLVL.IsValid());

    // MLVL Header
    rMLVL.WriteLong(0xDEAFBABE);
    rMLVL.WriteLong( GetMLVLVersion(pWorld->Game()) );

    CAssetID WorldNameID = pWorld->mpWorldName ? pWorld->mpWorldName->ID() : CAssetID::skInvalidID32;
    CAssetID SaveWorldID = pWorld->mpSaveWorld ? pWorld->mpSaveWorld->ID() : CAssetID::skInvalidID32;
    CAssetID DefaultSkyID = pWorld->mpDefaultSkybox ? pWorld->mpDefaultSkybox->ID() : CAssetID::skInvalidID32;

    WorldNameID.Write(rMLVL);
    SaveWorldID.Write(rMLVL);
    DefaultSkyID.Write(rMLVL);

    // Memory Relays
    rMLVL.WriteLong( pWorld->mMemoryRelays.size() );

    for (u32 iMem = 0; iMem < pWorld->mMemoryRelays.size(); iMem++)
    {
        CWorld::SMemoryRelay& rRelay = pWorld->mMemoryRelays[iMem];
        rMLVL.WriteLong(rRelay.InstanceID);
        rMLVL.WriteLong(rRelay.TargetID);
        rMLVL.WriteShort(rRelay.Message);
        rMLVL.WriteByte(rRelay.Unknown);
    }

    // Areas
    rMLVL.WriteLong(pWorld->mAreas.size());
    rMLVL.WriteLong(1); // Unknown

    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        // Area Header
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        CResourceEntry *pAreaEntry = gpResourceStore->FindEntry(rArea.AreaResID);
        ASSERT(pAreaEntry && pAreaEntry->ResourceType() == eArea);

        CAssetID AreaNameID = rArea.pAreaName ? rArea.pAreaName->ID() : CAssetID::skInvalidID32;
        AreaNameID.Write(rMLVL);
        rArea.Transform.Write(rMLVL);
        rArea.AetherBox.Write(rMLVL);
        rArea.AreaResID.Write(rMLVL);
        rMLVL.WriteLong( (u32) rArea.AreaID );

        // Attached Areas
        rMLVL.WriteLong( rArea.AttachedAreaIDs.size() );

        for (u32 iAttach = 0; iAttach < rArea.AttachedAreaIDs.size(); iAttach++)
            rMLVL.WriteShort(rArea.AttachedAreaIDs[iAttach]);

        // Dependencies
        std::list<CAssetID> Dependencies;
        std::list<u32> LayerDependsOffsets;
        CAreaDependencyListBuilder Builder(pAreaEntry);
        Builder.BuildDependencyList(Dependencies, LayerDependsOffsets);

        rMLVL.WriteLong(0);
        rMLVL.WriteLong( Dependencies.size() );

        for (auto Iter = Dependencies.begin(); Iter != Dependencies.end(); Iter++)
        {
            CAssetID ID = *Iter;
            CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
            ID.Write(rMLVL);
            pEntry->CookedExtension().Write(rMLVL);
        }

        rMLVL.WriteLong(LayerDependsOffsets.size());

        for (auto Iter = LayerDependsOffsets.begin(); Iter != LayerDependsOffsets.end(); Iter++)
            rMLVL.WriteLong(*Iter);

        // Docks
        rMLVL.WriteLong( rArea.Docks.size() );

        for (u32 iDock = 0; iDock < rArea.Docks.size(); iDock++)
        {
            CWorld::SArea::SDock& rDock = rArea.Docks[iDock];
            rMLVL.WriteLong( rDock.ConnectingDocks.size() );

            for (u32 iCon = 0; iCon < rDock.ConnectingDocks.size(); iCon++)
            {
                CWorld::SArea::SDock::SConnectingDock& rConDock = rDock.ConnectingDocks[iCon];
                rMLVL.WriteLong(rConDock.AreaIndex);
                rMLVL.WriteLong(rConDock.DockIndex);
            }

            rMLVL.WriteLong( rDock.DockCoordinates.size() );

            for (u32 iCoord = 0; iCoord < rDock.DockCoordinates.size(); iCoord++)
                rDock.DockCoordinates[iCoord].Write(rMLVL);
        }
    }

    CAssetID MapWorldID = pWorld->mpMapWorld ? pWorld->mpMapWorld->ID() : CAssetID::skInvalidID32;
    MapWorldID.Write(rMLVL);
    rMLVL.WriteByte(0);
    rMLVL.WriteLong(0);

    // Audio Groups
    rMLVL.WriteLong(pWorld->mAudioGrps.size());

    for (u32 iGrp = 0; iGrp < pWorld->mAudioGrps.size(); iGrp++)
    {
        CWorld::SAudioGrp& rAudioGroup = pWorld->mAudioGrps[iGrp];
        rMLVL.WriteLong(rAudioGroup.Unknown);
        rAudioGroup.ResID.Write(rMLVL);
    }

    rMLVL.WriteByte(0);

    // Layers
    rMLVL.WriteLong(pWorld->mAreas.size());
    std::vector<TString> LayerNames;
    std::vector<u32> LayerNameOffsets;

    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        LayerNameOffsets.push_back(LayerNames.size());
        rMLVL.WriteLong(rArea.Layers.size());

        u64 LayerActiveFlags = -1;

        for (u32 iLyr = 0; iLyr < rArea.Layers.size(); iLyr++)
        {
            CWorld::SArea::SLayer& rLayer = rArea.Layers[iLyr];
            if (!rLayer.EnabledByDefault)
                LayerActiveFlags &= ~(1 << iLyr);

            LayerNames.push_back(rLayer.LayerName);
        }

        rMLVL.WriteLongLong(LayerActiveFlags);
    }

    rMLVL.WriteLong(LayerNames.size());

    for (u32 iLyr = 0; iLyr < LayerNames.size(); iLyr++)
        rMLVL.WriteString(LayerNames[iLyr].ToStdString());

    rMLVL.WriteLong(LayerNameOffsets.size());

    for (u32 iOff = 0; iOff < LayerNameOffsets.size(); iOff++)
        rMLVL.WriteLong(LayerNameOffsets[iOff]);

    return true;
}

u32 CWorldCooker::GetMLVLVersion(EGame Version)
{
    switch (Version)
    {
    case ePrimeDemo:  return 0xD;
    case ePrime:      return 0x11;
    case eEchoesDemo: return 0x14;
    case eEchoes:     return 0x17;
    case eCorruption: return 0x19;
    case eReturns:    return 0x1B;
    default:          return 0;
    }
}
