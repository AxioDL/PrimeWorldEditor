#include "CWorldLoader.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CWorldLoader::CWorldLoader()
{
}

void CWorldLoader::LoadPrimeMLVL(IInputStream& rMLVL)
{
    /*
     * This function loads MLVL files from Prime 1/2
     * We start immediately after the "version" value (0x8 in the file)
     */
    // Header
    if (mVersion < EGame::CorruptionProto)
    {
        mpWorld->mpWorldName = gpResourceStore->LoadResource(rMLVL.ReadLong(), EResourceType::StringTable);
        if (mVersion == EGame::Echoes) mpWorld->mpDarkWorldName = gpResourceStore->LoadResource(rMLVL.ReadLong(), EResourceType::StringTable);
        if (mVersion >= EGame::Echoes) mpWorld->mTempleKeyWorldIndex = rMLVL.ReadLong();
        if (mVersion >= EGame::Prime) mpWorld->mpSaveWorld = gpResourceStore->LoadResource(rMLVL.ReadLong(), EResourceType::SaveWorld);
        mpWorld->mpDefaultSkybox = gpResourceStore->LoadResource(rMLVL.ReadLong(), EResourceType::Model);
    }

    else
    {
        mpWorld->mpWorldName = gpResourceStore->LoadResource(rMLVL.ReadLongLong(), EResourceType::StringTable);
        rMLVL.Seek(0x4, SEEK_CUR); // Skipping unknown value
        mpWorld->mpSaveWorld = gpResourceStore->LoadResource(rMLVL.ReadLongLong(), EResourceType::SaveWorld);
        mpWorld->mpDefaultSkybox = gpResourceStore->LoadResource(rMLVL.ReadLongLong(), EResourceType::Model);
    }

    // Memory relays - only in MP1
    if (mVersion == EGame::Prime)
    {
        uint32 NumMemoryRelays = rMLVL.ReadLong();
        mpWorld->mMemoryRelays.reserve(NumMemoryRelays);

        for (uint32 iMem = 0; iMem < NumMemoryRelays; iMem++)
        {
            CWorld::SMemoryRelay MemRelay;
            MemRelay.InstanceID = rMLVL.ReadLong();
            MemRelay.TargetID = rMLVL.ReadLong();
            MemRelay.Message = rMLVL.ReadShort();
            MemRelay.Active = rMLVL.ReadBool();
            mpWorld->mMemoryRelays.push_back(MemRelay);
        }
    }

    // Areas - here's the real meat of the file
    uint32 NumAreas = rMLVL.ReadLong();
    if (mVersion == EGame::Prime) rMLVL.Seek(0x4, SEEK_CUR);
    mpWorld->mAreas.resize(NumAreas);

    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];
        pArea->pAreaName = gpResourceStore->LoadResource<CStringTable>( CAssetID(rMLVL, mVersion) );
        pArea->Transform = CTransform4f(rMLVL);
        pArea->AetherBox = CAABox(rMLVL);
        pArea->AreaResID = CAssetID(rMLVL, mVersion);
        pArea->AreaID = CAssetID(rMLVL, mVersion);

        // Attached areas
        uint32 NumAttachedAreas = rMLVL.ReadLong();
        pArea->AttachedAreaIDs.reserve(NumAttachedAreas);
        for (uint32 iAttached = 0; iAttached < NumAttachedAreas; iAttached++)
            pArea->AttachedAreaIDs.push_back( rMLVL.ReadShort() );

        // Skip dependency list - this is very fast to regenerate so there's no use in caching it
        if (mVersion < EGame::CorruptionProto)
        {
            rMLVL.Seek(0x4, SEEK_CUR);
            uint32 NumDependencies = rMLVL.ReadLong();
            rMLVL.Seek(NumDependencies * 8, SEEK_CUR);

            uint32 NumDependencyOffsets = rMLVL.ReadLong();
            rMLVL.Seek(NumDependencyOffsets * 4, SEEK_CUR);
        }

        // Docks
        uint32 NumDocks = rMLVL.ReadLong();
        pArea->Docks.resize(NumDocks);

        for (uint32 iDock = 0; iDock < NumDocks; iDock++)
        {
            uint32 NumConnectingDocks = rMLVL.ReadLong();

            CWorld::SArea::SDock* pDock = &pArea->Docks[iDock];
            pDock->ConnectingDocks.reserve(NumConnectingDocks);

            for (uint32 iConnect = 0; iConnect < NumConnectingDocks; iConnect++)
            {
                CWorld::SArea::SDock::SConnectingDock ConnectingDock;
                ConnectingDock.AreaIndex = rMLVL.ReadLong();
                ConnectingDock.DockIndex = rMLVL.ReadLong();
                pDock->ConnectingDocks.push_back(ConnectingDock);
            }

            uint32 NumCoordinates = rMLVL.ReadLong();
            ASSERT(NumCoordinates == 4);
            pDock->DockCoordinates.resize(NumCoordinates);

            for (uint32 iCoord = 0; iCoord < NumCoordinates; iCoord++)
                pDock->DockCoordinates[iCoord] = CVector3f(rMLVL);
        }

        // Rels
        if ( (mVersion == EGame::EchoesDemo) || (mVersion == EGame::Echoes) )
        {
            uint32 NumRels = rMLVL.ReadLong();
            pArea->RelFilenames.resize(NumRels);

            for (uint32 iRel = 0; iRel < NumRels; iRel++)
                pArea->RelFilenames[iRel] = rMLVL.ReadString();

            if (mVersion == EGame::Echoes)
            {
                uint32 NumRelOffsets = rMLVL.ReadLong(); // Don't know what these offsets correspond to
                pArea->RelOffsets.resize(NumRelOffsets);

                for (uint32 iOff = 0; iOff < NumRelOffsets; iOff++)
                    pArea->RelOffsets[iOff] = rMLVL.ReadLong();
            }
        }

        // Internal name - MP1 doesn't have this, we'll get it from the GameInfo file later
        if (mVersion >= EGame::EchoesDemo)
            pArea->InternalName = rMLVL.ReadString();
    }

    // MapWorld
    mpWorld->mpMapWorld = gpResourceStore->LoadResource( CAssetID(rMLVL, mVersion), EResourceType::MapWorld );
    rMLVL.Seek(0x5, SEEK_CUR); // Unknown values which are always 0

    // Audio Groups - we don't need this info as we regenerate it on cook
    if (mVersion == EGame::Prime)
    {
        uint32 NumAudioGrps = rMLVL.ReadLong();
        rMLVL.Seek(0x8 * NumAudioGrps, SEEK_CUR);
        rMLVL.Seek(0x1, SEEK_CUR); // Unknown values which are always 0
    }

    // Layer flags
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count
    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        uint32 NumLayers = rMLVL.ReadLong();
        if (NumLayers != pArea->Layers.size()) pArea->Layers.resize(NumLayers);

        uint64 LayerFlags = rMLVL.ReadLongLong();
        for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].Active = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        uint32 NumLayers = pArea->Layers.size();

        for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = rMLVL.ReadString();
    }

    // Layer state IDs
    if (mVersion >= EGame::Corruption)
    {
        rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
        for (uint32 iArea = 0; iArea < NumAreas; iArea++)
        {
            CWorld::SArea *pArea = &mpWorld->mAreas[iArea];
            uint32 NumLayers = pArea->Layers.size();

            for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
                pArea->Layers[iLayer].LayerStateID = CSavedStateID(rMLVL);
        }
    }

    // Last part of the file is layer name offsets, but we don't need it
}

void CWorldLoader::LoadReturnsMLVL(IInputStream& rMLVL)
{
    mpWorld->mpWorldName = gpResourceStore->LoadResource<CStringTable>(rMLVL.ReadLongLong());

    CWorld::STimeAttackData& rData = mpWorld->mTimeAttackData;
    rData.HasTimeAttack = rMLVL.ReadBool();

    if (rData.HasTimeAttack)
    {
        rData.ActNumber = rMLVL.ReadString();
        rData.BronzeTime = rMLVL.ReadFloat();
        rData.SilverTime = rMLVL.ReadFloat();
        rData.GoldTime = rMLVL.ReadFloat();
        rData.ShinyGoldTime = rMLVL.ReadFloat();
    }

    mpWorld->mpSaveWorld = gpResourceStore->LoadResource(rMLVL.ReadLongLong(), EResourceType::SaveWorld);
    mpWorld->mpDefaultSkybox = gpResourceStore->LoadResource<CModel>(rMLVL.ReadLongLong());

    // Areas
    uint32 NumAreas = rMLVL.ReadLong();
    mpWorld->mAreas.resize(NumAreas);

    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];

        pArea->pAreaName = gpResourceStore->LoadResource<CStringTable>(rMLVL.ReadLongLong());
        pArea->Transform = CTransform4f(rMLVL);
        pArea->AetherBox = CAABox(rMLVL);
        pArea->AreaResID = rMLVL.ReadLongLong();
        pArea->AreaID = rMLVL.ReadLongLong();

        rMLVL.Seek(0x4, SEEK_CUR);
        pArea->InternalName = rMLVL.ReadString();
    }

    // Layer flags
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count

    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        uint32 NumLayers = rMLVL.ReadLong();
        pArea->Layers.resize(NumLayers);

        uint64 LayerFlags = rMLVL.ReadLongLong();
        for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].Active = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        uint32 NumLayers = pArea->Layers.size();

        for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = rMLVL.ReadString();
    }

    // Layer state IDs
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];
        uint32 NumLayers = pArea->Layers.size();

        for (uint32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerStateID = CSavedStateID(rMLVL);
    }

    // Last part of the file is layer name offsets, but we don't need it
}

void CWorldLoader::GenerateEditorData()
{
    const CGameInfo *pGameInfo = mpWorld->Entry()->ResourceStore()->Project()->GameInfo();

    if (mVersion > EGame::Prime)
        return;

    for (size_t iArea = 0; iArea < mpWorld->NumAreas(); iArea++)
    {
        CWorld::SArea& rArea = mpWorld->mAreas[iArea];
        rArea.InternalName = pGameInfo->GetAreaName(rArea.AreaResID);
        ASSERT(!rArea.InternalName.IsEmpty());
    }
}

std::unique_ptr<CWorld> CWorldLoader::LoadMLVL(IInputStream& rMLVL, CResourceEntry *pEntry)
{
    if (!rMLVL.IsValid()) return nullptr;

    uint32 Magic = rMLVL.ReadLong();
    if (Magic != 0xDEAFBABE)
    {
        errorf("%s: Invalid MLVL magic: 0x%08X", *rMLVL.GetSourceString(), Magic);
        return nullptr;
    }

    uint32 FileVersion = rMLVL.ReadLong();
    EGame Version = GetFormatVersion(FileVersion);
    if (Version == EGame::Invalid)
    {
        errorf("%s: Unsupported MLVL version: 0x%X", *rMLVL.GetSourceString(), FileVersion);
        return nullptr;
    }

    // Filestream is valid, magic+version are valid; everything seems good!
    auto ptr = std::make_unique<CWorld>(pEntry);

    CWorldLoader Loader;
    Loader.mpWorld = ptr.get();
    Loader.mVersion = Version;

    if (Version != EGame::DKCReturns)
        Loader.LoadPrimeMLVL(rMLVL);
    else
        Loader.LoadReturnsMLVL(rMLVL);

    Loader.GenerateEditorData();
    return ptr;
}

EGame CWorldLoader::GetFormatVersion(uint32 Version)
{
    switch (Version)
    {
        case 0xD: return EGame::PrimeDemo;
        case 0x11: return EGame::Prime;
        case 0x14: return EGame::EchoesDemo;
        case 0x17: return EGame::Echoes;
        case 0x19: return EGame::Corruption;
        case 0x1B: return EGame::DKCReturns;
        default: return EGame::Invalid;
    }
}
