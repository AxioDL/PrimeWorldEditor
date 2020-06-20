#include "CWorldCooker.h"
#include "Core/GameProject/DependencyListBuilders.h"

CWorldCooker::CWorldCooker() = default;

// ************ STATIC ************
bool CWorldCooker::CookMLVL(CWorld *pWorld, IOutputStream& rMLVL)
{
    ASSERT(rMLVL.IsValid());
    const EGame Game = pWorld->Game();

    // MLVL Header
    rMLVL.WriteULong(0xDEAFBABE);
    rMLVL.WriteULong(GetMLVLVersion(pWorld->Game()));

    const CAssetID WorldNameID = pWorld->mpWorldName != nullptr ? pWorld->mpWorldName->ID() : CAssetID::InvalidID(Game);
    const CAssetID DarkWorldNameID = pWorld->mpDarkWorldName != nullptr ? pWorld->mpDarkWorldName->ID() : CAssetID::InvalidID(Game);
    const CAssetID SaveWorldID = pWorld->mpSaveWorld != nullptr ? pWorld->mpSaveWorld->ID() : CAssetID::InvalidID(Game);
    const CAssetID DefaultSkyID = pWorld->mpDefaultSkybox != nullptr ? pWorld->mpDefaultSkybox->ID() : CAssetID::InvalidID(Game);

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
        rMLVL.WriteULong(static_cast<uint32>(pWorld->mMemoryRelays.size()));

        for (const auto& relay : pWorld->mMemoryRelays)
        {
            rMLVL.WriteULong(relay.InstanceID);
            rMLVL.WriteULong(relay.TargetID);
            rMLVL.WriteUShort(relay.Message);
            rMLVL.WriteBool(relay.Active);
        }
    }

    // Areas
    rMLVL.WriteULong(static_cast<uint32>(pWorld->mAreas.size()));
    if (Game <= EGame::Prime)
        rMLVL.WriteULong(1); // Unknown
    std::set<CAssetID> AudioGroups;

    for (auto& rArea : pWorld->mAreas)
    {
        // Area Header
        CResourceEntry *pAreaEntry = gpResourceStore->FindEntry(rArea.AreaResID);
        ASSERT(pAreaEntry && pAreaEntry->ResourceType() == EResourceType::Area);

        const CAssetID AreaNameID = rArea.pAreaName != nullptr ? rArea.pAreaName->ID() : CAssetID::InvalidID(Game);
        AreaNameID.Write(rMLVL);
        rArea.Transform.Write(rMLVL);
        rArea.AetherBox.Write(rMLVL);
        rArea.AreaResID.Write(rMLVL);
        rArea.AreaID.Write(rMLVL);

        // Attached Areas
        if (Game <= EGame::Corruption)
        {
            rMLVL.WriteULong(static_cast<uint32>(rArea.AttachedAreaIDs.size()));

            for (const auto id : rArea.AttachedAreaIDs)
                rMLVL.WriteUShort(id);
        }

        // Dependencies
        if (Game <= EGame::Echoes)
        {
            std::list<CAssetID> Dependencies;
            std::list<uint32> LayerDependsOffsets;
            CAreaDependencyListBuilder Builder(pAreaEntry);
            Builder.BuildDependencyList(Dependencies, LayerDependsOffsets, &AudioGroups);

            rMLVL.WriteULong(0);
            rMLVL.WriteULong(static_cast<uint32>(Dependencies.size()));

            for (const auto& ID : Dependencies)
            {
                CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
                ID.Write(rMLVL);
                pEntry->CookedExtension().Write(rMLVL);
            }

            rMLVL.WriteLong(static_cast<uint32>(LayerDependsOffsets.size()));

            for (const auto offset : LayerDependsOffsets)
                rMLVL.WriteULong(offset);
        }

        // Docks
        if (Game <= EGame::Corruption)
        {
            rMLVL.WriteULong(static_cast<uint32>(rArea.Docks.size()));

            for (const auto& dock : rArea.Docks)
            {
                rMLVL.WriteULong(static_cast<uint32>(dock.ConnectingDocks.size()));

                for (const auto& connectingDock : dock.ConnectingDocks)
                {
                    rMLVL.WriteULong(connectingDock.AreaIndex);
                    rMLVL.WriteULong(connectingDock.DockIndex);
                }

                rMLVL.WriteULong(static_cast<uint32>(dock.DockCoordinates.size()));

                for (const auto& coordinate : dock.DockCoordinates)
                    coordinate.Write(rMLVL);
            }
        }

        // Module Dependencies
        if (Game == EGame::EchoesDemo || Game == EGame::Echoes)
        {
            std::vector<TString> ModuleNames;
            std::vector<uint32> ModuleLayerOffsets;
            const auto *pAreaDeps = static_cast<CAreaDependencyTree*>(pAreaEntry->Dependencies());
            pAreaDeps->GetModuleDependencies(Game, ModuleNames, ModuleLayerOffsets);

            rMLVL.WriteULong(static_cast<uint32>(ModuleNames.size()));

            for (const auto& name : ModuleNames)
                rMLVL.WriteString(name);

            rMLVL.WriteULong(static_cast<uint32>(ModuleLayerOffsets.size()));

            for (const auto offset : ModuleLayerOffsets)
                rMLVL.WriteULong(offset);
        }

        // Unknown
        if (Game == EGame::DKCReturns)
            rMLVL.WriteULong(0);

        // Internal Name
        if (Game >= EGame::EchoesDemo)
            rMLVL.WriteString(rArea.InternalName);
    }

    if (Game <= EGame::Corruption)
    {
        // World Map
        const CAssetID MapWorldID = pWorld->mpMapWorld != nullptr ? pWorld->mpMapWorld->ID() : CAssetID::skInvalidID32;
        MapWorldID.Write(rMLVL);

        // Script Layer - unused in all retail builds but this will need to be supported eventually to properly support the MP1 demo
        rMLVL.WriteUByte(0);
        rMLVL.WriteULong(0);
    }

    // Audio Groups
    if (Game <= EGame::Prime)
    {
        // Create sorted list of audio groups (sort by group ID)
        std::vector<CAudioGroup*> SortedAudioGroups;

        for (const auto AudioGroup : AudioGroups)
        {
            CAudioGroup *pGroup = gpResourceStore->LoadResource<CAudioGroup>(AudioGroup);
            ASSERT(pGroup);
            SortedAudioGroups.push_back(pGroup);
        }

        std::sort(SortedAudioGroups.begin(), SortedAudioGroups.end(), [](const auto* pLeft, const auto* pRight) {
            return pLeft->GroupID() < pRight->GroupID();
        });

        // Write sorted audio group list to file
        rMLVL.WriteULong(static_cast<uint32>(SortedAudioGroups.size()));

        for (const auto* pGroup : SortedAudioGroups)
        {
            rMLVL.WriteULong(pGroup->GroupID());
            pGroup->ID().Write(rMLVL);
        }

        rMLVL.WriteUByte(0);
    }

    // Layers
    rMLVL.WriteLong(pWorld->mAreas.size());
    std::vector<TString> LayerNames;
    std::vector<CSavedStateID> LayerStateIDs;
    std::vector<uint32> LayerNameOffsets;

    // Layer Flags
    for (const auto& rArea : pWorld->mAreas)
    {
        LayerNameOffsets.push_back(LayerNames.size());
        rMLVL.WriteULong(static_cast<uint32>(rArea.Layers.size()));

        uint64 LayerActiveFlags = UINT64_MAX;

        for (uint32 iLyr = 0; iLyr < rArea.Layers.size(); iLyr++)
        {
            const CWorld::SArea::SLayer& rLayer = rArea.Layers[iLyr];
            if (!rLayer.Active)
                LayerActiveFlags &= ~(UINT64_C(1) << iLyr);

            LayerNames.push_back(rLayer.LayerName);
            LayerStateIDs.push_back(rLayer.LayerStateID);
        }

        rMLVL.WriteULongLong(LayerActiveFlags);
    }

    // Layer Names
    rMLVL.WriteULong(static_cast<uint32>(LayerNames.size()));

    for (const auto& name : LayerNames)
        rMLVL.WriteString(name);

    // Layer Saved State IDs
    if (Game >= EGame::Corruption)
    {
        rMLVL.WriteULong(static_cast<uint32>(LayerStateIDs.size()));

        for (auto& id : LayerStateIDs)
            id.Write(rMLVL);
    }

    // Layer Name Offsets
    rMLVL.WriteULong(static_cast<uint32>(LayerNameOffsets.size()));

    for (const auto offset : LayerNameOffsets)
        rMLVL.WriteLong(offset);

    return true;
}

uint32 CWorldCooker::GetMLVLVersion(EGame Version)
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
