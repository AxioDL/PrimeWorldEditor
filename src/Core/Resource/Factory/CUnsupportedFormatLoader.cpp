#include "CUnsupportedFormatLoader.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"
#include "Core/Resource/CWorld.h"

void CUnsupportedFormatLoader::PerformCheating(IInputStream& rFile, EGame Game, std::list<CAssetID>& rAssetList)
{
    // Analyze file contents and check every sequence of 4/8 bytes for asset IDs
    std::vector<uint8> Data(rFile.Size() - rFile.Tell());
    rFile.ReadBytes(Data.data(), Data.size());

    const uint32 MaxIndex = (Game <= EGame::Echoes ? Data.size() - 3 : Data.size() - 7);
    CAssetID ID;

    for (uint32 iByte = 0; iByte < MaxIndex; iByte++)
    {
        if (Game <= EGame::Echoes)
        {
            ID = (Data[iByte + 0] << 24) |
                 (Data[iByte + 1] << 16) |
                 (Data[iByte + 2] << 8) |
                 (Data[iByte + 3] << 0);
        }
        else
        {
            ID = (static_cast<uint64>(Data[iByte + 0]) << 56) |
                 (static_cast<uint64>(Data[iByte + 1]) << 48) |
                 (static_cast<uint64>(Data[iByte + 2]) << 40) |
                 (static_cast<uint64>(Data[iByte + 3]) << 32) |
                 (static_cast<uint64>(Data[iByte + 4]) << 24) |
                 (static_cast<uint64>(Data[iByte + 5]) << 16) |
                 (static_cast<uint64>(Data[iByte + 6]) << 8) |
                 (static_cast<uint64>(Data[iByte + 7]) << 0);
        }

        if (gpResourceStore->IsResourceRegistered(ID))
            rAssetList.push_back(ID);
    }
}

std::unique_ptr<CAudioMacro> CUnsupportedFormatLoader::LoadCAUD(IInputStream& rCAUD, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rCAUD.ReadULong();
    ASSERT(Magic == FOURCC('CAUD'));

    const uint32 Version = rCAUD.ReadLong();
    const EGame Game = Version == 0x2 ? EGame::CorruptionProto :
                       Version == 0x9 ? EGame::Corruption :
                       Version == 0xE ? EGame::DKCReturns :
                       EGame::Invalid;
    ASSERT(Game != EGame::Invalid && Game == pEntry->Game());

    auto pMacro = std::make_unique<CAudioMacro>(pEntry);
    pMacro->mMacroName = rCAUD.ReadString();

    // DKCR is missing the sample data size value, and the bulk of the format isn't well understood, unfortunately
    if (Game == EGame::DKCReturns)
    {
        std::list<CAssetID> AssetList;
        PerformCheating(rCAUD, pEntry->Game(), AssetList);

        for (const auto& asset : AssetList)
            pMacro->mSamples.push_back(asset);

        return pMacro;
    }

    // Skip past the rest of the header
    const uint32 NumVolGroups = rCAUD.ReadULong();

    for (uint32 iVol = 0; iVol < NumVolGroups; iVol++)
        rCAUD.ReadString();

    const uint32 SkipAmt = Game == EGame::CorruptionProto ? 0x10 : 0x14;
    rCAUD.Seek(SkipAmt, SEEK_CUR);
    const uint32 NumSamples = rCAUD.ReadULong();

    for (uint32 iSamp = 0; iSamp < NumSamples; iSamp++)
    {
        const uint32 SampleDataSize = rCAUD.ReadULong();
        const uint32 SampleDataEnd = rCAUD.Tell() + SampleDataSize;

        const CAssetID SampleID(rCAUD, Game);
        ASSERT(gpResourceStore->IsResourceRegistered(SampleID) == true);
        pMacro->mSamples.push_back(SampleID);

        rCAUD.Seek(SampleDataEnd, SEEK_SET);
    }

    return pMacro;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadCSNG(IInputStream& rCSNG, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rCSNG.ReadULong();
    ASSERT(Magic == 0x2);
    rCSNG.Seek(0x8, SEEK_CUR);

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    pGroup->AddDependency(rCSNG.ReadULong());
    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadDUMB(IInputStream& rDUMB, CResourceEntry *pEntry)
{
    // Check for HIER, which needs special handling
    if (rDUMB.PeekLong() == FOURCC('HIER'))
        return LoadHIER(rDUMB, pEntry);

    // Load other DUMB file. DUMB files don't have a set format - they're different between different files
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    std::list<CAssetID> DepList;
    PerformCheating(rDUMB, pEntry->Game(), DepList);

    for (const auto& dep : DepList)
        pGroup->AddDependency(dep);

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadFRME(IInputStream& rFRME, CResourceEntry *pEntry)
{
    const uint32 Version = rFRME.ReadULong();
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    // Prime 1
    if (Version == 0 || Version == 1)
    {
        rFRME.Seek(0xC, SEEK_CUR);
        const uint32 NumWidgets = rFRME.ReadULong();

        for (uint32 iWgt = 0; iWgt < NumWidgets; iWgt++)
        {
            // Widget Header
            CFourCC WidgetType = rFRME.ReadULong();
            rFRME.ReadString();
            rFRME.ReadString();
            rFRME.Seek(0x18, SEEK_CUR);

            // Head Widget / Base Widget
            if (WidgetType == FOURCC('HWIG') || WidgetType == FOURCC('BWIG'))
            {
            }
            // Camera
            else if (WidgetType == FOURCC('CAMR'))
            {
                const uint32 ProjectionType = rFRME.ReadULong();

                if (ProjectionType == 0)
                    rFRME.Seek(0x10, SEEK_CUR);
                else
                    rFRME.Seek(0x18, SEEK_CUR);
            }
            // Light
            else if (WidgetType == FOURCC('LITE'))
            {
                const uint32 LightType = rFRME.ReadULong();
                rFRME.Seek(0x1C, SEEK_CUR);
                if (LightType == 0)
                    rFRME.Seek(0x4, SEEK_CUR);
            }
            // Meter
            else if (WidgetType == FOURCC('METR'))
            {
                rFRME.Seek(0xA, SEEK_CUR);
            }
            // Group
            else if (WidgetType == FOURCC('GRUP'))
            {
                rFRME.Seek(0x3, SEEK_CUR);
            }
            // Table Group
            else if (WidgetType == FOURCC('TBGP'))
            {
                rFRME.Seek(0x23, SEEK_CUR);
            }
            // Model
            else if (WidgetType == FOURCC('MODL'))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // CMDL
                rFRME.Seek(0x8, SEEK_CUR);
            }
            // Text Pane
            else if (WidgetType == FOURCC('TXPN'))
            {
                rFRME.Seek(0x14, SEEK_CUR);
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // FONT
                rFRME.Seek(0x32, SEEK_CUR);

                if (Version == 1)
                {
                    pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // FONT
                    rFRME.Seek(0x8, SEEK_CUR);
                }
            }
            // Image Pane
            else if (WidgetType == FOURCC('IMGP'))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // TXTR
                if (rFRME.ReadULong() != 0xFFFFFFFF)
                    DEBUG_BREAK;
                rFRME.Seek(0x4, SEEK_CUR);

                const uint32 NumQuadCoords = rFRME.ReadULong();
                rFRME.Seek(NumQuadCoords * 0xC, SEEK_CUR);
                const uint32 NumUVCoords = rFRME.ReadULong();
                rFRME.Seek(NumUVCoords * 8, SEEK_CUR);
            }
            // Energy Bar
            else if (WidgetType == FOURCC('ENRG'))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // TXTR
            }
            // Slider Group
            else if (WidgetType == FOURCC('SLGP'))
            {
                rFRME.Seek(0x10, SEEK_CUR);
            }
            else
            {
                errorf("Unrecognized FRME widget type: %s", *WidgetType.ToString());
                DEBUG_BREAK;
            }

            // Widget Footer
            if (rFRME.ReadByte() != 0)
                rFRME.Seek(0x2, SEEK_CUR);

            rFRME.Seek(0x42, SEEK_CUR);
        }
    }
    // MP2/MP3/DKCR are much easier... dependency list right at the beginning of the file
    else if (Version == 4 || Version == 5 || Version == 0xD || Version == 0xE || Version == 0x10)
    {
        EGame Game;
        if (Version == 4)
            Game = EGame::Echoes;
        else if (Version == 0x10)
            Game = EGame::DKCReturns;
        else
            Game = EGame::Corruption;

        const uint32 NumDependencies = rFRME.ReadULong();

        for (uint32 iDep = 0; iDep < NumDependencies; iDep++)
        {
            rFRME.Seek(0x4, SEEK_CUR);
            pGroup->AddDependency(CAssetID(rFRME, Game));
        }
    }
    else
    {
        errorf("Unrecognized FRME version: %d", Version);
        return nullptr;
    }

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadFSM2(IInputStream& rFSM2, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rFSM2.ReadULong();
    ASSERT(Magic == FOURCC('FSM2'));

    auto pOut = std::make_unique<CDependencyGroup>(pEntry);
    const uint32 Version = rFSM2.ReadULong();
    const uint32 NumStates = rFSM2.ReadULong();
    const uint32 NumUnkA = rFSM2.ReadULong();
    const uint32 NumUnkB = rFSM2.ReadULong();
    const uint32 NumUnkC = rFSM2.ReadULong();
    ASSERT(Version == 1 || Version == 2);

    for (uint32 iState = 0; iState < NumStates; iState++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);

        const uint32 UnkCount = rFSM2.ReadULong();

        for (uint32 iUnk = 0; iUnk < UnkCount; iUnk++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32 iUnkA = 0; iUnkA < NumUnkA; iUnkA++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);
        rFSM2.Seek(0x4, SEEK_CUR);
        const uint32 UnkCount = rFSM2.ReadULong();

        for (uint32 iUnkA2 = 0; iUnkA2 < UnkCount; iUnkA2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }

        rFSM2.Seek(0x1, SEEK_CUR);
    }

    for (uint32 iUnkB = 0; iUnkB < NumUnkB; iUnkB++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);
        const uint32 UnkCount = rFSM2.ReadULong();

        for (uint32 iUnkB2 = 0; iUnkB2 < UnkCount; iUnkB2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32 iUnkC = 0; iUnkC < NumUnkC; iUnkC++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);
        const uint32 UnkCount = rFSM2.ReadLong();

        for (uint32 iUnkC2 = 0; iUnkC2 < UnkCount; iUnkC2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }

        pOut->AddDependency(CAssetID(rFSM2, pEntry->Game()));
    }

    return pOut;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadFSMC(IInputStream& rFSMC, CResourceEntry *pEntry)
{
    [[maybe_unused]] const CFourCC Magic = rFSMC.ReadULong();
    ASSERT(Magic == FOURCC('FSMC'));

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    std::list<CAssetID> AssetList;
    PerformCheating(rFSMC, pEntry->Game(), AssetList);

    for (const auto& asset : AssetList)
        pGroup->AddDependency(asset);

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadHIER(IInputStream& rHIER, CResourceEntry *pEntry)
{
    [[maybe_unused]] const CFourCC Magic = rHIER.ReadLong();
    ASSERT(Magic == "HIER");

    const uint32 NumNodes = rHIER.ReadULong();
    auto pOut = std::make_unique<CDependencyGroup>(pEntry);

    // Note: For some reason this file still exists in MP3 and it's identical to MP2, including with 32-bit asset IDs.
    // Obviously we can't read 32-bit asset IDs in MP3, so this file should just be ignored.
    if (pEntry->Game() > EGame::Echoes)
        return pOut;

    for (uint32 iNode = 0; iNode < NumNodes; iNode++)
    {
        // NOTE: The SCAN ID isn't considered a real dependency!
        pOut->AddDependency(rHIER.ReadULong());
        rHIER.ReadString();
        rHIER.Seek(0x8, SEEK_CUR);
    }

    return pOut;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadHINT(IInputStream& rHINT, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rHINT.ReadULong();
    ASSERT(Magic == 0x00BADBAD);

    // Determine version
    const uint32 Version = rHINT.ReadULong();
    EGame Game;

    if (Version == 0x1)
    {
        Game = EGame::Prime;
    }
    else if (Version == 0x3)
    {
        Game = EGame::Corruption;
    }
    else
    {
        errorf("Unrecognized HINT version: %d", Version);
        return nullptr;
    }

    // Read main file
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    const uint32 NumHints = rHINT.ReadULong();

    for (uint32 iHint = 0; iHint < NumHints; iHint++)
    {
        rHINT.ReadString(); // Skip hint name
        rHINT.Seek(0x8, SEEK_CUR); // Skip unknown + appear time
        pGroup->AddDependency(CAssetID(rHINT, Game)); // Pop-up STRG
        rHINT.Seek(0x4, SEEK_CUR); // Skip unknowns

        if (Game <= EGame::Echoes)
        {
            rHINT.Seek(0x4, SEEK_CUR);
            pGroup->AddDependency(CAssetID(rHINT, Game)); // Target MLVL
            pGroup->AddDependency(CAssetID(rHINT, Game)); // Target MREA
            rHINT.Seek(0x4, SEEK_CUR); // Skip target room index
            pGroup->AddDependency(CAssetID(rHINT, Game)); // Map STRG
        }
        else
        {
            const uint32 NumLocations = rHINT.ReadULong();

            for (uint32 iLoc = 0; iLoc < NumLocations; iLoc++)
            {
                rHINT.Seek(0x14, SEEK_CUR); // Skip world/area ID, area index
                pGroup->AddDependency(CAssetID(rHINT, Game)); // Objective string
                rHINT.Seek(0xC, SEEK_CUR); // Skip unknown data
            }
        }
    }

    return pGroup;
}

std::unique_ptr<CMapArea> CUnsupportedFormatLoader::LoadMAPA(IInputStream& /*rMAPA*/, CResourceEntry *pEntry)
{
    auto ptr = std::make_unique<CMapArea>(pEntry);
    TResPtr<CMapArea> pMap = ptr.get();

    // We don't actually read the file. Just fetch the name string so we can build the dependency tree.
    const CAssetID MapAreaID = pMap->ID();

    // Find a MapWorld that contains this MapArea
    CAssetID MapWorldID;
    size_t WorldIndex = SIZE_MAX;

    for (TResourceIterator<EResourceType::MapWorld> It; It; ++It)
    {
        auto *pGroup = static_cast<CDependencyGroup*>(It->Load());

        for (size_t AreaIdx = 0; AreaIdx < pGroup->NumDependencies(); AreaIdx++)
        {
            if (pGroup->DependencyByIndex(AreaIdx) == MapAreaID)
            {
                WorldIndex = AreaIdx;
                MapWorldID = pGroup->ID();
                break;
            }
        }

        if (WorldIndex != SIZE_MAX)
            break;
    }

    // Find a world that contains this MapWorld
    if (WorldIndex != SIZE_MAX)
    {
        for (TResourceIterator<EResourceType::World> It; It; ++It)
        {
            auto *pWorld = static_cast<CWorld*>(It->Load());
            auto *pMapWorld = static_cast<CDependencyGroup*>(pWorld->MapWorld());

            if (pMapWorld != nullptr && pMapWorld->ID() == MapWorldID)
            {
                CStringTable *pNameString = pWorld->AreaName(WorldIndex);
                pMap->mNameString = (pNameString != nullptr ? pNameString->ID() : CAssetID::InvalidID(pEntry->Game()));
                break;
            }
        }
    }

    pMap->Entry()->ResourceStore()->DestroyUnreferencedResources();
    return ptr;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadMAPW(IInputStream& rMAPW, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rMAPW.ReadULong();
    ASSERT(Magic == 0xDEADF00D);

    [[maybe_unused]] const uint32 Version = rMAPW.ReadULong();
    ASSERT(Version == 1);

    const uint32 NumAreas = rMAPW.ReadULong();

    // Version check
    const uint32 AreasStart = rMAPW.Tell();
    rMAPW.Seek(NumAreas * 4, SEEK_CUR);
    const auto IDLength = (rMAPW.EoF() || rMAPW.ReadULong() == 0xFFFFFFFF ? EIDLength::k32Bit : EIDLength::k64Bit);
    rMAPW.Seek(AreasStart, SEEK_SET);

    // Read MAPA IDs
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
        pGroup->AddDependency(CAssetID(rMAPW, IDLength));

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadMAPU(IInputStream& rMAPU, CResourceEntry *pEntry)
{
    [[maybe_unused]] const uint32 Magic = rMAPU.ReadULong();
    ASSERT(Magic == 0xABCDEF01);

    [[maybe_unused]] const uint32 Version = rMAPU.ReadULong();
    ASSERT(Version == 0x1);

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    pGroup->AddDependency(rMAPU.ReadULong());

    // Read worlds
    const uint32 NumWorlds = rMAPU.ReadULong();

    for (uint32 iWorld = 0; iWorld < NumWorlds; iWorld++)
    {
        rMAPU.ReadString(); // Skip world name
        pGroup->AddDependency(rMAPU.ReadULong()); // World MLVL
        rMAPU.Seek(0x30, SEEK_CUR); // Skip world map transform
        const uint32 NumHexagons = rMAPU.ReadULong();
        rMAPU.Seek(NumHexagons * 0x30, SEEK_CUR); // Skip hexagon transforms
        rMAPU.Seek(0x10, SEEK_CUR); // Skip world color
    }

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry)
{
    // RULE files can contain a reference to another RULE file, but has no other dependencies.
    [[maybe_unused]] const uint32 Magic = rRULE.ReadULong();
    ASSERT(Magic == FOURCC('RULE'));

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    rRULE.Seek(0x1, SEEK_CUR);

    // Version test
    const uint32 IDOffset = rRULE.Tell();
    rRULE.Seek(0x4, SEEK_CUR);
    const uint32 RuleSetCount = rRULE.ReadUShort();
    const auto IDLength = (RuleSetCount > 0xFF ? EIDLength::k64Bit : EIDLength::k32Bit);
    rRULE.Seek(IDOffset, SEEK_SET);

    // Read rule ID
    const CAssetID RuleID(rRULE, IDLength);
    pGroup->AddDependency(RuleID);
    return pGroup;
}
