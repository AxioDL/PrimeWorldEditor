#include "CWorldCooker.h"
#include "Core/GameProject/DependencyListBuilders.h"

CWorldCooker::CWorldCooker()
{
}

// ************ STATIC ************
bool CWorldCooker::CookMLVL(CWorld *pWorld, IOutputStream& rMLVL)
{
    ASSERT(rMLVL.IsValid());
    EGame Game = pWorld->Game();

    // MLVL Header
    rMLVL.WriteLong(0xDEAFBABE);
    rMLVL.WriteLong( GetMLVLVersion(pWorld->Game()) );

    CAssetID WorldNameID = pWorld->mpWorldName ? pWorld->mpWorldName->ID() : CAssetID::InvalidID(Game);
    CAssetID DarkWorldNameID = pWorld->mpDarkWorldName ? pWorld->mpDarkWorldName->ID() : CAssetID::InvalidID(Game);
    CAssetID SaveWorldID = pWorld->mpSaveWorld ? pWorld->mpSaveWorld->ID() : CAssetID::InvalidID(Game);
    CAssetID DefaultSkyID = pWorld->mpDefaultSkybox ? pWorld->mpDefaultSkybox->ID() : CAssetID::InvalidID(Game);

    WorldNameID.Write(rMLVL);

    if (Game == EGame::EchoesDemo || Game == EGame::Echoes)
    {
        DarkWorldNameID.Write(rMLVL);
    }
    if (Game >= EGame::EchoesDemo && Game <= EGame::Corruption)
    {
        rMLVL.WriteLong(pWorld->mTempleKeyWorldIndex);
    }
    if (Game == EGame::DKCReturns)
    {
        const CWorld::STimeAttackData& rkData = pWorld->mTimeAttackData;
        rMLVL.WriteBool(rkData.HasTimeAttack);

        if (rkData.HasTimeAttack)
        {
            rMLVL.WriteString(rkData.ActNumber);
            rMLVL.WriteFloat(rkData.BronzeTime);
            rMLVL.WriteFloat(rkData.SilverTime);
            rMLVL.WriteFloat(rkData.GoldTime);
            rMLVL.WriteFloat(rkData.ShinyGoldTime);
        }
    }

    SaveWorldID.Write(rMLVL);
    DefaultSkyID.Write(rMLVL);

    // Memory Relays
    if (Game == EGame::Prime)
    {
        rMLVL.WriteLong( pWorld->mMemoryRelays.size() );

        for (u32 iMem = 0; iMem < pWorld->mMemoryRelays.size(); iMem++)
        {
            CWorld::SMemoryRelay& rRelay = pWorld->mMemoryRelays[iMem];
            rMLVL.WriteLong(rRelay.InstanceID);
            rMLVL.WriteLong(rRelay.TargetID);
            rMLVL.WriteShort(rRelay.Message);
            rMLVL.WriteBool(rRelay.Active);
        }
    }

    // Areas
    rMLVL.WriteLong(pWorld->mAreas.size());
    if (Game <= EGame::Prime) rMLVL.WriteLong(1); // Unknown
    std::set<CAssetID> AudioGroups;

    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        // Area Header
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        CResourceEntry *pAreaEntry = gpResourceStore->FindEntry(rArea.AreaResID);
        ASSERT(pAreaEntry && pAreaEntry->ResourceType() == eArea);

        CAssetID AreaNameID = rArea.pAreaName ? rArea.pAreaName->ID() : CAssetID::InvalidID(Game);
        AreaNameID.Write(rMLVL);
        rArea.Transform.Write(rMLVL);
        rArea.AetherBox.Write(rMLVL);
        rArea.AreaResID.Write(rMLVL);
        rArea.AreaID.Write(rMLVL);

        // Attached Areas
        if (Game <= EGame::Corruption)
        {
            rMLVL.WriteLong( rArea.AttachedAreaIDs.size() );

            for (u32 iAttach = 0; iAttach < rArea.AttachedAreaIDs.size(); iAttach++)
                rMLVL.WriteShort(rArea.AttachedAreaIDs[iAttach]);
        }

        // Dependencies
        if (Game <= EGame::Echoes)
        {
            std::list<CAssetID> Dependencies;
            std::list<u32> LayerDependsOffsets;
            CAreaDependencyListBuilder Builder(pAreaEntry);
            Builder.BuildDependencyList(Dependencies, LayerDependsOffsets, &AudioGroups);

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
        }

        // Docks
        if (Game <= EGame::Corruption)
        {
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

        // Module Dependencies
        if (Game == EGame::EchoesDemo || Game == EGame::Echoes)
        {
            std::vector<TString> ModuleNames;
            std::vector<u32> ModuleLayerOffsets;
            CAreaDependencyTree *pAreaDeps = static_cast<CAreaDependencyTree*>(pAreaEntry->Dependencies());
            pAreaDeps->GetModuleDependencies(Game, ModuleNames, ModuleLayerOffsets);

            rMLVL.WriteLong(ModuleNames.size());

            for (u32 iMod = 0; iMod < ModuleNames.size(); iMod++)
                rMLVL.WriteString(ModuleNames[iMod]);

            rMLVL.WriteLong(ModuleLayerOffsets.size());

            for (u32 iOff = 0; iOff < ModuleLayerOffsets.size(); iOff++)
                rMLVL.WriteLong(ModuleLayerOffsets[iOff]);
        }

        // Unknown
        if (Game == EGame::DKCReturns)
            rMLVL.WriteLong(0);

        // Internal Name
        if (Game >= EGame::EchoesDemo)
            rMLVL.WriteString(rArea.InternalName);
    }

    if (Game <= EGame::Corruption)
    {
        // World Map
        CAssetID MapWorldID = pWorld->mpMapWorld ? pWorld->mpMapWorld->ID() : CAssetID::skInvalidID32;
        MapWorldID.Write(rMLVL);

        // Script Layer - unused in all retail builds but this will need to be supported eventually to properly support the MP1 demo
        rMLVL.WriteByte(0);
        rMLVL.WriteLong(0);
    }

    // Audio Groups
    if (Game <= EGame::Prime)
    {
        // Create sorted list of audio groups (sort by group ID)
        std::vector<CAudioGroup*> SortedAudioGroups;

        for (auto It = AudioGroups.begin(); It != AudioGroups.end(); It++)
        {
            CAudioGroup *pGroup = gpResourceStore->LoadResource<CAudioGroup>(*It);
            ASSERT(pGroup);
            SortedAudioGroups.push_back(pGroup);
        }

        std::sort(SortedAudioGroups.begin(), SortedAudioGroups.end(), [](CAudioGroup *pLeft, CAudioGroup *pRight) -> bool {
            return pLeft->GroupID() < pRight->GroupID();
        });

        // Write sorted audio group list to file
        rMLVL.WriteLong(SortedAudioGroups.size());

        for (u32 iGrp = 0; iGrp < SortedAudioGroups.size(); iGrp++)
        {
            CAudioGroup *pGroup = SortedAudioGroups[iGrp];
            rMLVL.WriteLong(pGroup->GroupID());
            pGroup->ID().Write(rMLVL);
        }

        rMLVL.WriteByte(0);
    }

    // Layers
    rMLVL.WriteLong(pWorld->mAreas.size());
    std::vector<TString> LayerNames;
    std::vector<CSavedStateID> LayerStateIDs;
    std::vector<u32> LayerNameOffsets;

    // Layer Flags
    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        LayerNameOffsets.push_back(LayerNames.size());
        rMLVL.WriteLong(rArea.Layers.size());

        u64 LayerActiveFlags = -1;

        for (u32 iLyr = 0; iLyr < rArea.Layers.size(); iLyr++)
        {
            CWorld::SArea::SLayer& rLayer = rArea.Layers[iLyr];
            if (!rLayer.Active)
                LayerActiveFlags &= ~(1 << iLyr);

            LayerNames.push_back(rLayer.LayerName);
            LayerStateIDs.push_back(rLayer.LayerStateID);
        }

        rMLVL.WriteLongLong(LayerActiveFlags);
    }

    // Layer Names
    rMLVL.WriteLong(LayerNames.size());

    for (u32 iLyr = 0; iLyr < LayerNames.size(); iLyr++)
        rMLVL.WriteString(LayerNames[iLyr]);

    // Layer Saved State IDs
    if (Game >= EGame::Corruption)
    {
        rMLVL.WriteLong(LayerStateIDs.size());

        for (u32 iLyr = 0; iLyr < LayerStateIDs.size(); iLyr++)
            LayerStateIDs[iLyr].Write(rMLVL);
    }

    // Layer Name Offsets
    rMLVL.WriteLong(LayerNameOffsets.size());

    for (u32 iOff = 0; iOff < LayerNameOffsets.size(); iOff++)
        rMLVL.WriteLong(LayerNameOffsets[iOff]);

    return true;
}

u32 CWorldCooker::GetMLVLVersion(EGame Version)
{
    switch (Version)
    {
    case EGame::PrimeDemo:  return 0xD;
    case EGame::Prime:      return 0x11;
    case EGame::EchoesDemo: return 0x14;
    case EGame::Echoes:     return 0x17;
    case EGame::Corruption: return 0x19;
    case EGame::DKCReturns:    return 0x1B;
    default:          return 0;
    }
}
