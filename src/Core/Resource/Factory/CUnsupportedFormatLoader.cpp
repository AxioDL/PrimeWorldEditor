#include "Core/Resource/Factory/CUnsupportedFormatLoader.h"

#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/Log.h>
#include "Core/NRangeUtils.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/Resource/CAudioMacro.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CMapArea.h"
#include "Core/Resource/CWorld.h"

#include <list>

static void PerformCheating(IInputStream& rFile, EGame Game, std::list<CAssetID>& rAssetList, const CResourceStore* resourceStore)
{
    // Analyze file contents and check every sequence of 4/8 bytes for asset IDs
    std::vector<uint8_t> Data(rFile.Size() - rFile.Tell());
    rFile.ReadBytes(Data.data(), Data.size());

    const size_t MaxIndex = (Game <= EGame::Echoes ? Data.size() - 3 : Data.size() - 7);
    CAssetID ID;

    for (size_t iByte = 0; iByte < MaxIndex; iByte++)
    {
        if (Game <= EGame::Echoes)
        {
            ID = (uint32_t{Data[iByte + 0]} << 24) |
                 (uint32_t{Data[iByte + 1]} << 16) |
                 (uint32_t{Data[iByte + 2]} << 8) |
                 (uint32_t{Data[iByte + 3]} << 0);
        }
        else
        {
            ID = (uint64_t{Data[iByte + 0]} << 56) |
                 (uint64_t{Data[iByte + 1]} << 48) |
                 (uint64_t{Data[iByte + 2]} << 40) |
                 (uint64_t{Data[iByte + 3]} << 32) |
                 (uint64_t{Data[iByte + 4]} << 24) |
                 (uint64_t{Data[iByte + 5]} << 16) |
                 (uint64_t{Data[iByte + 6]} << 8) |
                 (uint64_t{Data[iByte + 7]} << 0);
        }

        if (resourceStore->IsResourceRegistered(ID))
            rAssetList.push_back(ID);
    }
}

std::unique_ptr<CAudioMacro> CUnsupportedFormatLoader::LoadCAUD(IInputStream& rCAUD, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rCAUD.ReadFourCC();
    ASSERT(Magic == CFourCC("CAUD"));

    const auto Version = rCAUD.ReadU32();
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
        PerformCheating(rCAUD, pEntry->Game(), AssetList, pEntry->ResourceStore());

        pMacro->mSamples.append_range(AssetList);
        return pMacro;
    }

    // Skip past the rest of the header
    const auto NumVolGroups = rCAUD.ReadU32();

    for (uint32_t iVol = 0; iVol < NumVolGroups; iVol++)
        rCAUD.ReadString();

    const auto SkipAmt = Game == EGame::CorruptionProto ? 0x10U : 0x14U;
    rCAUD.Seek(SkipAmt, SEEK_CUR);
    const auto NumSamples = rCAUD.ReadU32();

    for (uint32_t iSamp = 0; iSamp < NumSamples; iSamp++)
    {
        const auto SampleDataSize = rCAUD.ReadU32();
        const auto SampleDataEnd = rCAUD.Tell() + SampleDataSize;

        const CAssetID SampleID(rCAUD, Game);
        ASSERT(pEntry->ResourceStore()->IsResourceRegistered(SampleID) == true);
        pMacro->mSamples.push_back(SampleID);

        rCAUD.Seek(SampleDataEnd, SEEK_SET);
    }

    return pMacro;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadCSNG(IInputStream& rCSNG, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rCSNG.ReadU32();
    ASSERT(Magic == 0x2);
    rCSNG.Seek(0x8, SEEK_CUR);

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    pGroup->AddDependency(rCSNG.ReadU32());
    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadDUMB(IInputStream& rDUMB, CResourceEntry *pEntry)
{
    // Check for HIER, which needs special handling
    if (rDUMB.PeekFourCC() == CFourCC("HIER"))
        return LoadHIER(rDUMB, pEntry);

    // Load other DUMB file. DUMB files don't have a set format - they're different between different files
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    std::list<CAssetID> DepList;
    PerformCheating(rDUMB, pEntry->Game(), DepList, pEntry->ResourceStore());

    for (const auto& dep : DepList)
        pGroup->AddDependency(dep);

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadFRME(IInputStream& rFRME, CResourceEntry *pEntry)
{
    const auto Version = rFRME.ReadU32();
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    // Prime 1
    if (Version == 0 || Version == 1)
    {
        rFRME.Seek(0xC, SEEK_CUR);
        const auto NumWidgets = rFRME.ReadU32();

        for (uint32_t iWgt = 0; iWgt < NumWidgets; iWgt++)
        {
            // Widget Header
            const CFourCC WidgetType = rFRME.ReadFourCC();
            rFRME.ReadString();
            rFRME.ReadString();
            rFRME.Seek(0x18, SEEK_CUR);

            // Head Widget / Base Widget
            if (WidgetType == CFourCC("HWIG") || WidgetType == CFourCC("BWIG"))
            {
            }
            // Camera
            else if (WidgetType == CFourCC("CAMR"))
            {
                const auto ProjectionType = rFRME.ReadU32();

                if (ProjectionType == 0)
                    rFRME.Seek(0x10, SEEK_CUR);
                else
                    rFRME.Seek(0x18, SEEK_CUR);
            }
            // Light
            else if (WidgetType == CFourCC("LITE"))
            {
                const auto LightType = rFRME.ReadU32();
                rFRME.Seek(0x1C, SEEK_CUR);
                if (LightType == 0)
                    rFRME.Seek(0x4, SEEK_CUR);
            }
            // Meter
            else if (WidgetType == CFourCC("METR"))
            {
                rFRME.Seek(0xA, SEEK_CUR);
            }
            // Group
            else if (WidgetType == CFourCC("GRUP"))
            {
                rFRME.Seek(0x3, SEEK_CUR);
            }
            // Table Group
            else if (WidgetType == CFourCC("TBGP"))
            {
                rFRME.Seek(0x23, SEEK_CUR);
            }
            // Model
            else if (WidgetType == CFourCC("MODL"))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // CMDL
                rFRME.Seek(0x8, SEEK_CUR);
            }
            // Text Pane
            else if (WidgetType == CFourCC("TXPN"))
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
            else if (WidgetType == CFourCC("IMGP"))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // TXTR
                if (rFRME.ReadU32() != 0xFFFFFFFF)
                    DEBUG_BREAK;
                rFRME.Seek(0x4, SEEK_CUR);

                const auto NumQuadCoords = rFRME.ReadU32();
                rFRME.Seek(NumQuadCoords * 0xC, SEEK_CUR);
                const auto NumUVCoords = rFRME.ReadU32();
                rFRME.Seek(NumUVCoords * 8, SEEK_CUR);
            }
            // Energy Bar
            else if (WidgetType == CFourCC("ENRG"))
            {
                pGroup->AddDependency(CAssetID(rFRME, EIDLength::k32Bit)); // TXTR
            }
            // Slider Group
            else if (WidgetType == CFourCC("SLGP"))
            {
                rFRME.Seek(0x10, SEEK_CUR);
            }
            else
            {
                NLog::Error("Unrecognized FRME widget type: {}", WidgetType.ToString());
                DEBUG_BREAK;
            }

            // Widget Footer
            if (rFRME.ReadS8() != 0)
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

        const auto NumDependencies = rFRME.ReadU32();

        for (uint32_t iDep = 0; iDep < NumDependencies; iDep++)
        {
            rFRME.Seek(0x4, SEEK_CUR);
            pGroup->AddDependency(CAssetID(rFRME, Game));
        }
    }
    else
    {
        NLog::Error("Unrecognized FRME version: {}", Version);
        return nullptr;
    }

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadFSM2(IInputStream& rFSM2, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rFSM2.ReadFourCC();
    ASSERT(Magic == CFourCC("FSM2"));

    auto pOut = std::make_unique<CDependencyGroup>(pEntry);
    const auto Version = rFSM2.ReadU32();
    const auto NumStates = rFSM2.ReadU32();
    const auto NumUnkA = rFSM2.ReadU32();
    const auto NumUnkB = rFSM2.ReadU32();
    const auto NumUnkC = rFSM2.ReadU32();
    ASSERT(Version == 1 || Version == 2);

    for (uint32_t iState = 0; iState < NumStates; iState++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);

        const auto UnkCount = rFSM2.ReadU32();
        for (uint32_t iUnk = 0; iUnk < UnkCount; iUnk++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32_t iUnkA = 0; iUnkA < NumUnkA; iUnkA++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);
        rFSM2.Seek(0x4, SEEK_CUR);

        const auto UnkCount = rFSM2.ReadU32();
        for (uint32_t iUnkA2 = 0; iUnkA2 < UnkCount; iUnkA2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }

        rFSM2.Seek(0x1, SEEK_CUR);
    }

    for (uint32_t iUnkB = 0; iUnkB < NumUnkB; iUnkB++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);

        const auto UnkCount = rFSM2.ReadU32();
        for (uint32_t iUnkB2 = 0; iUnkB2 < UnkCount; iUnkB2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32_t iUnkC = 0; iUnkC < NumUnkC; iUnkC++)
    {
        rFSM2.ReadString();
        if (Version >= 2)
            rFSM2.Seek(0x10, SEEK_CUR);

        const auto UnkCount = rFSM2.ReadU32();
        for (uint32_t iUnkC2 = 0; iUnkC2 < UnkCount; iUnkC2++)
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
    [[maybe_unused]] const auto Magic = rFSMC.ReadFourCC();
    ASSERT(Magic == CFourCC("FSMC"));

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    std::list<CAssetID> AssetList;
    PerformCheating(rFSMC, pEntry->Game(), AssetList, pEntry->ResourceStore());

    for (const auto& asset : AssetList)
        pGroup->AddDependency(asset);

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadHIER(IInputStream& rHIER, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rHIER.ReadFourCC();
    ASSERT(Magic == CFourCC("HIER"));

    const auto NumNodes = rHIER.ReadU32();
    auto pOut = std::make_unique<CDependencyGroup>(pEntry);

    // Note: For some reason this file still exists in MP3 and it's identical to MP2, including with 32-bit asset IDs.
    // Obviously we can't read 32-bit asset IDs in MP3, so this file should just be ignored.
    if (pEntry->Game() > EGame::Echoes)
        return pOut;

    for (uint32_t iNode = 0; iNode < NumNodes; iNode++)
    {
        // NOTE: The SCAN ID isn't considered a real dependency!
        pOut->AddDependency(rHIER.ReadU32());
        rHIER.ReadString();
        rHIER.Seek(0x8, SEEK_CUR);
    }

    return pOut;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadHINT(IInputStream& rHINT, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rHINT.ReadU32();
    ASSERT(Magic == 0x00BADBAD);

    // Determine version
    const auto Version = rHINT.ReadU32();
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
        NLog::Error("Unrecognized HINT version: {}", Version);
        return nullptr;
    }

    // Read main file
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    const auto NumHints = rHINT.ReadU32();

    for (uint32_t iHint = 0; iHint < NumHints; iHint++)
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
            const auto NumLocations = rHINT.ReadU32();

            for (uint32_t iLoc = 0; iLoc < NumLocations; iLoc++)
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
    const CAssetID& MapAreaID = pMap->ID();

    // Find a MapWorld that contains this MapArea
    CAssetID MapWorldID;
    size_t WorldIndex = SIZE_MAX;

    for (const auto& It : pEntry->ResourceStore()->MakeTypedResourceView(EResourceType::MapWorld))
    {
        auto *pGroup = static_cast<CDependencyGroup*>(It->Load());

        for (const auto&& [idx, dep] : Utils::enumerate(pGroup->Dependencies()))
        {
            if (dep == MapAreaID)
            {
                WorldIndex = idx;
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
        for (const auto& It : pEntry->ResourceStore()->MakeTypedResourceView(EResourceType::World))
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
    [[maybe_unused]] const auto Magic = rMAPW.ReadU32();
    ASSERT(Magic == 0xDEADF00D);

    [[maybe_unused]] const auto Version = rMAPW.ReadU32();
    ASSERT(Version == 1);

    const auto NumAreas = rMAPW.ReadU32();

    // Version check
    const auto AreasStart = rMAPW.Tell();
    rMAPW.Seek(NumAreas * 4, SEEK_CUR);
    const auto IDLength = (rMAPW.EoF() || rMAPW.ReadU32() == 0xFFFFFFFF ? EIDLength::k32Bit : EIDLength::k64Bit);
    rMAPW.Seek(AreasStart, SEEK_SET);

    // Read MAPA IDs
    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    for (uint32_t iArea = 0; iArea < NumAreas; iArea++)
        pGroup->AddDependency(CAssetID(rMAPW, IDLength));

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadMAPU(IInputStream& rMAPU, CResourceEntry *pEntry)
{
    [[maybe_unused]] const auto Magic = rMAPU.ReadU32();
    ASSERT(Magic == 0xABCDEF01);

    [[maybe_unused]] const auto Version = rMAPU.ReadU32();
    ASSERT(Version == 0x1);

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    pGroup->AddDependency(rMAPU.ReadU32());

    // Read worlds
    const auto NumWorlds = rMAPU.ReadU32();

    for (uint32_t iWorld = 0; iWorld < NumWorlds; iWorld++)
    {
        rMAPU.ReadString(); // Skip world name
        pGroup->AddDependency(rMAPU.ReadU32()); // World MLVL
        rMAPU.Seek(0x30, SEEK_CUR); // Skip world map transform
        const auto NumHexagons = rMAPU.ReadU32();
        rMAPU.Seek(NumHexagons * 0x30, SEEK_CUR); // Skip hexagon transforms
        rMAPU.Seek(0x10, SEEK_CUR); // Skip world color
    }

    return pGroup;
}

std::unique_ptr<CDependencyGroup> CUnsupportedFormatLoader::LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry)
{
    // RULE files can contain a reference to another RULE file, but has no other dependencies.
    [[maybe_unused]] const auto Magic = rRULE.ReadFourCC();
    ASSERT(Magic == CFourCC("RULE"));

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);
    rRULE.Seek(0x1, SEEK_CUR);

    // Version test
    const auto IDOffset = rRULE.Tell();
    rRULE.Seek(0x4, SEEK_CUR);
    const auto RuleSetCount = rRULE.ReadU16();
    const auto IDLength = (RuleSetCount > 0xFF ? EIDLength::k64Bit : EIDLength::k32Bit);
    rRULE.Seek(IDOffset, SEEK_SET);

    // Read rule ID
    const CAssetID RuleID(rRULE, IDLength);
    pGroup->AddDependency(RuleID);
    return pGroup;
}
