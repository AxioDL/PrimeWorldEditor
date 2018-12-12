#include "CUnsupportedFormatLoader.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/GameProject/CResourceIterator.h"
#include "Core/Resource/CWorld.h"

void CUnsupportedFormatLoader::PerformCheating(IInputStream& rFile, EGame Game, std::list<CAssetID>& rAssetList)
{
    // Analyze file contents and check every sequence of 4/8 bytes for asset IDs
    std::vector<uint8> Data(rFile.Size() - rFile.Tell());
    rFile.ReadBytes(Data.data(), Data.size());

    uint32 MaxIndex = (Game <= EGame::Echoes ? Data.size() - 3 : Data.size() - 7);
    CAssetID ID;

    for (uint32 iByte = 0; iByte < MaxIndex; iByte++)
    {
        if (Game <= EGame::Echoes)
        {
            ID = ( (Data[iByte+0] << 24) |
                   (Data[iByte+1] << 16) |
                   (Data[iByte+2] <<  8) |
                   (Data[iByte+3] <<  0) );
        }
        else
        {
            ID = ( ((uint64) Data[iByte+0] << 56) |
                   ((uint64) Data[iByte+1] << 48) |
                   ((uint64) Data[iByte+2] << 40) |
                   ((uint64) Data[iByte+3] << 32) |
                   ((uint64) Data[iByte+4] << 24) |
                   ((uint64) Data[iByte+5] << 16) |
                   ((uint64) Data[iByte+6] <<  8) |
                   ((uint64) Data[iByte+7] <<  0) );
        }

        if (gpResourceStore->IsResourceRegistered(ID))
            rAssetList.push_back(ID);
    }
}

CAudioMacro* CUnsupportedFormatLoader::LoadCAUD(IInputStream& rCAUD, CResourceEntry *pEntry)
{
    uint32 Magic = rCAUD.ReadLong();
    ASSERT(Magic == FOURCC('CAUD'));

    uint32 Version = rCAUD.ReadLong();
    EGame Game = (Version == 0x2 ? EGame::CorruptionProto :
                  Version == 0x9 ? EGame::Corruption :
                  Version == 0xE ? EGame::DKCReturns :
                  EGame::Invalid);
    ASSERT(Game != EGame::Invalid && Game == pEntry->Game());

    CAudioMacro *pMacro = new CAudioMacro(pEntry);
    pMacro->mMacroName = rCAUD.ReadString();

    // DKCR is missing the sample data size value, and the bulk of the format isn't well understood, unfortunately
    if (Game == EGame::DKCReturns)
    {
        std::list<CAssetID> AssetList;
        PerformCheating(rCAUD, pEntry->Game(), AssetList);

        for (auto Iter = AssetList.begin(); Iter != AssetList.end(); Iter++)
            pMacro->mSamples.push_back(*Iter);

        return pMacro;
    }

    // Skip past the rest of the header
    uint32 NumVolGroups = rCAUD.ReadLong();

    for (uint32 iVol = 0; iVol < NumVolGroups; iVol++)
        rCAUD.ReadString();

    uint32 SkipAmt = (Game == EGame::CorruptionProto ? 0x10 : 0x14);
    rCAUD.Seek(SkipAmt, SEEK_CUR);
    uint32 NumSamples = rCAUD.ReadLong();

    for (uint32 iSamp = 0; iSamp < NumSamples; iSamp++)
    {
        uint32 SampleDataSize = rCAUD.ReadLong();
        uint32 SampleDataEnd = rCAUD.Tell() + SampleDataSize;

        CAssetID SampleID(rCAUD, Game);
        ASSERT(gpResourceStore->IsResourceRegistered(SampleID) == true);
        pMacro->mSamples.push_back(SampleID);

        rCAUD.Seek(SampleDataEnd, SEEK_SET);
    }

    return pMacro;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadCSNG(IInputStream& rCSNG, CResourceEntry *pEntry)
{
    uint32 Magic = rCSNG.ReadLong();
    ASSERT(Magic == 0x2);
    rCSNG.Seek(0x8, SEEK_CUR);

    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);
    pGroup->AddDependency(rCSNG.ReadLong());
    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadDUMB(IInputStream& rDUMB, CResourceEntry *pEntry)
{
    // Check for HIER, which needs special handling
    if (rDUMB.PeekLong() == FOURCC('HIER'))
        return LoadHIER(rDUMB, pEntry);

    // Load other DUMB file. DUMB files don't have a set format - they're different between different files
    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);

    std::list<CAssetID> DepList;
    PerformCheating(rDUMB, pEntry->Game(), DepList);

    for (auto Iter = DepList.begin(); Iter != DepList.end(); Iter++)
        pGroup->AddDependency(*Iter);

    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadFRME(IInputStream& rFRME, CResourceEntry *pEntry)
{
    uint32 Version = rFRME.ReadLong();
    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);

    // Prime 1
    if (Version == 0 || Version == 1)
    {
        rFRME.Seek(0xC, SEEK_CUR);
        uint32 NumWidgets = rFRME.ReadLong();

        for (uint32 iWgt = 0; iWgt < NumWidgets; iWgt++)
        {
            // Widget Header
            CFourCC WidgetType = rFRME.ReadLong();
            rFRME.ReadString();
            rFRME.ReadString();
            rFRME.Seek(0x18, SEEK_CUR);

            // Head Widget / Base Widget
            if (WidgetType == FOURCC('HWIG') || WidgetType == FOURCC('BWIG'))
            {}

            // Camera
            else if (WidgetType == FOURCC('CAMR'))
            {
                uint32 ProjectionType = rFRME.ReadLong();

                if (ProjectionType == 0)
                    rFRME.Seek(0x10, SEEK_CUR);
                else
                    rFRME.Seek(0x18, SEEK_CUR);
            }

            // Light
            else if (WidgetType == FOURCC('LITE'))
            {
                uint32 LightType = rFRME.ReadLong();
                rFRME.Seek(0x1C, SEEK_CUR);
                if (LightType == 0) rFRME.Seek(0x4, SEEK_CUR);
            }

            // Meter
            else if (WidgetType == FOURCC('METR'))
                rFRME.Seek(0xA, SEEK_CUR);

            // Group
            else if (WidgetType == FOURCC('GRUP'))
                rFRME.Seek(0x3, SEEK_CUR);

            // Table Group
            else if (WidgetType == FOURCC('TBGP'))
                rFRME.Seek(0x23, SEEK_CUR);

            // Model
            else if (WidgetType == FOURCC('MODL'))
            {
                pGroup->AddDependency( CAssetID(rFRME, e32Bit) ); // CMDL
                rFRME.Seek(0x8, SEEK_CUR);
            }

            // Text Pane
            else if (WidgetType == FOURCC('TXPN'))
            {
                rFRME.Seek(0x14, SEEK_CUR);
                pGroup->AddDependency( CAssetID(rFRME, e32Bit) ); // FONT
                rFRME.Seek(0x32, SEEK_CUR);

                if (Version == 1)
                {
                    pGroup->AddDependency( CAssetID(rFRME, e32Bit) ); // FONT
                    rFRME.Seek(0x8, SEEK_CUR);
                }
            }

            // Image Pane
            else if (WidgetType == FOURCC('IMGP'))
            {
                pGroup->AddDependency( CAssetID(rFRME, e32Bit) ); // TXTR
                if (rFRME.ReadLong() != 0xFFFFFFFF) DEBUG_BREAK;
                rFRME.Seek(0x4, SEEK_CUR);

                uint32 NumQuadCoords = rFRME.ReadLong();
                rFRME.Seek(NumQuadCoords * 0xC, SEEK_CUR);
                uint32 NumUVCoords = rFRME.ReadLong();
                rFRME.Seek(NumUVCoords * 8, SEEK_CUR);
            }

            // Energy Bar
            else if (WidgetType == FOURCC('ENRG'))
            {
                pGroup->AddDependency( CAssetID(rFRME, e32Bit) ); // TXTR
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
        if (Version == 4)           Game = EGame::Echoes;
        else if (Version == 0x10)   Game = EGame::DKCReturns;
        else                        Game = EGame::Corruption;

        uint32 NumDependencies = rFRME.ReadLong();

        for (uint32 iDep = 0; iDep < NumDependencies; iDep++)
        {
            rFRME.Seek(0x4, SEEK_CUR);
            pGroup->AddDependency( CAssetID(rFRME, Game) );
        }
    }

    else
    {
        errorf("Unrecognized FRME version: %d", Version);
        delete pGroup;
        return nullptr;
    }

    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadFSM2(IInputStream& rFSM2, CResourceEntry *pEntry)
{
    uint32 Magic = rFSM2.ReadLong();
    ASSERT(Magic == FOURCC('FSM2'));

    CDependencyGroup *pOut = new CDependencyGroup(pEntry);
    uint32 Version = rFSM2.ReadLong();
    uint32 NumStates = rFSM2.ReadLong();
    uint32 NumUnkA = rFSM2.ReadLong();
    uint32 NumUnkB = rFSM2.ReadLong();
    uint32 NumUnkC = rFSM2.ReadLong();
    ASSERT(Version == 1 || Version == 2);

    for (uint32 iState = 0; iState < NumStates; iState++)
    {
        rFSM2.ReadString();
        if (Version >= 2) rFSM2.Seek(0x10, SEEK_CUR);
        uint32 UnkCount = rFSM2.ReadLong();

        for (uint32 iUnk = 0; iUnk < UnkCount; iUnk++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32 iUnkA = 0; iUnkA < NumUnkA; iUnkA++)
    {
        rFSM2.ReadString();
        if (Version >= 2) rFSM2.Seek(0x10, SEEK_CUR);
        rFSM2.Seek(0x4, SEEK_CUR);
        uint32 UnkCount = rFSM2.ReadLong();

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
        if (Version >= 2) rFSM2.Seek(0x10, SEEK_CUR);
        uint32 UnkCount = rFSM2.ReadLong();

        for (uint32 iUnkB2 = 0; iUnkB2 < UnkCount; iUnkB2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }
    }

    for (uint32 iUnkC = 0; iUnkC < NumUnkC; iUnkC++)
    {
        rFSM2.ReadString();
        if (Version >= 2) rFSM2.Seek(0x10, SEEK_CUR);
        uint32 UnkCount = rFSM2.ReadLong();

        for (uint32 iUnkC2 = 0; iUnkC2 < UnkCount; iUnkC2++)
        {
            rFSM2.ReadString();
            rFSM2.Seek(0x4, SEEK_CUR);
        }

        pOut->AddDependency( CAssetID(rFSM2, pEntry->Game()) );
    }

    return pOut;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadFSMC(IInputStream& rFSMC, CResourceEntry *pEntry)
{
    CFourCC Magic = rFSMC.ReadLong();
    ASSERT(Magic == FOURCC('FSMC'));

    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);

    std::list<CAssetID> AssetList;
    PerformCheating(rFSMC, pEntry->Game(), AssetList);

    for (auto Iter = AssetList.begin(); Iter != AssetList.end(); Iter++)
        pGroup->AddDependency(*Iter);

    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadHIER(IInputStream& rHIER, CResourceEntry *pEntry)
{
    CFourCC Magic = rHIER.ReadLong();
    ASSERT(Magic == "HIER");

    uint32 NumNodes = rHIER.ReadLong();
    CDependencyGroup *pOut = new CDependencyGroup(pEntry);

    // Note: For some reason this file still exists in MP3 and it's identical to MP2, including with 32-bit asset IDs.
    // Obviously we can't read 32-bit asset IDs in MP3, so this file should just be ignored.
    if (pEntry->Game() > EGame::Echoes)
        return pOut;

    for (uint32 iNode = 0; iNode < NumNodes; iNode++)
    {
        // NOTE: The SCAN ID isn't considered a real dependency!
        pOut->AddDependency( rHIER.ReadLong() );
        rHIER.ReadString();
        rHIER.Seek(0x8, SEEK_CUR);
    }

    return pOut;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadHINT(IInputStream& rHINT, CResourceEntry *pEntry)
{
    uint32 Magic = rHINT.ReadLong();
    ASSERT(Magic == 0x00BADBAD);

    // Determine version
    uint32 Version = rHINT.ReadLong();
    EGame Game;

    if (Version == 0x1) Game = EGame::Prime;
    else if (Version == 0x3) Game = EGame::Corruption;

    else
    {
        errorf("Unrecognized HINT version: %d", Version);
        return nullptr;
    }

    // Read main file
    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);
    uint32 NumHints = rHINT.ReadLong();

    for (uint32 iHint = 0; iHint < NumHints; iHint++)
    {
        rHINT.ReadString(); // Skip hint name
        rHINT.Seek(0x8, SEEK_CUR); // Skip unknown + appear time
        pGroup->AddDependency( CAssetID(rHINT, Game) ); // Pop-up STRG
        rHINT.Seek(0x4, SEEK_CUR); // Skip unknowns

        if (Game <= EGame::Echoes)
        {
            rHINT.Seek(0x4, SEEK_CUR);
            pGroup->AddDependency( CAssetID(rHINT, Game) ); // Target MLVL
            pGroup->AddDependency( CAssetID(rHINT, Game) ); // Target MREA
            rHINT.Seek(0x4, SEEK_CUR); // Skip target room index
            pGroup->AddDependency( CAssetID(rHINT, Game) ); // Map STRG
        }

        else
        {
            uint32 NumLocations = rHINT.ReadLong();

            for (uint32 iLoc = 0; iLoc < NumLocations; iLoc++)
            {
                rHINT.Seek(0x14, SEEK_CUR); // Skip world/area ID, area index
                pGroup->AddDependency( CAssetID(rHINT, Game) ); // Objective string
                rHINT.Seek(0xC, SEEK_CUR); // Skip unknown data
            }
        }
    }

    return pGroup;
}

CMapArea* CUnsupportedFormatLoader::LoadMAPA(IInputStream& /*rMAPA*/, CResourceEntry *pEntry)
{
    TResPtr<CMapArea> pMap = new CMapArea(pEntry);

    // We don't actually read the file. Just fetch the name string so we can build the dependency tree.
    CAssetID MapAreaID = pMap->ID();

    // Find a MapWorld that contains this MapArea
    CAssetID MapWorldID;
    uint32 WorldIndex = -1;

    for (TResourceIterator<eMapWorld> It; It; ++It)
    {
        CDependencyGroup *pGroup = (CDependencyGroup*) It->Load();

        for (uint32 AreaIdx = 0; AreaIdx < pGroup->NumDependencies(); AreaIdx++)
        {
            if (pGroup->DependencyByIndex(AreaIdx) == MapAreaID)
            {
                WorldIndex = AreaIdx;
                MapWorldID = pGroup->ID();
                break;
            }
        }

        if (WorldIndex != -1)
            break;
    }

    // Find a world that contains this MapWorld
    if (WorldIndex != -1)
    {
        for (TResourceIterator<eWorld> It; It; ++It)
        {
            CWorld *pWorld = (CWorld*) It->Load();
            CDependencyGroup *pMapWorld = (CDependencyGroup*) pWorld->MapWorld();

            if (pMapWorld && pMapWorld->ID() == MapWorldID)
            {
                CStringTable *pNameString = pWorld->AreaName(WorldIndex);
                pMap->mNameString = (pNameString ? pNameString->ID() : CAssetID::InvalidID(pEntry->Game()));
                break;
            }
        }
    }

    pMap->Entry()->ResourceStore()->DestroyUnreferencedResources();
    return pMap;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadMAPW(IInputStream& rMAPW, CResourceEntry *pEntry)
{
    uint32 Magic = rMAPW.ReadLong();
    ASSERT(Magic == 0xDEADF00D);

    uint32 Version = rMAPW.ReadLong();
    ASSERT(Version == 1);

    uint32 NumAreas = rMAPW.ReadLong();

    // Version check
    uint32 AreasStart = rMAPW.Tell();
    rMAPW.Seek(NumAreas * 4, SEEK_CUR);
    EIDLength IDLength = (rMAPW.EoF() || rMAPW.ReadLong() == 0xFFFFFFFF ? e32Bit : e64Bit);
    rMAPW.Seek(AreasStart, SEEK_SET);

    // Read MAPA IDs
    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);

    for (uint32 iArea = 0; iArea < NumAreas; iArea++)
        pGroup->AddDependency( CAssetID(rMAPW, IDLength) );

    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadMAPU(IInputStream& rMAPU, CResourceEntry *pEntry)
{
    uint32 Magic = rMAPU.ReadLong();
    ASSERT(Magic == 0xABCDEF01);

    uint32 Version = rMAPU.ReadLong();
    ASSERT(Version == 0x1);

    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);
    pGroup->AddDependency(rMAPU.ReadLong());

    // Read worlds
    uint32 NumWorlds = rMAPU.ReadLong();

    for (uint32 iWorld = 0; iWorld < NumWorlds; iWorld++)
    {
        rMAPU.ReadString(); // Skip world name
        pGroup->AddDependency(rMAPU.ReadLong()); // World MLVL
        rMAPU.Seek(0x30, SEEK_CUR); // Skip world map transform
        uint32 NumHexagons = rMAPU.ReadLong();
        rMAPU.Seek(NumHexagons * 0x30, SEEK_CUR); // Skip hexagon transforms
        rMAPU.Seek(0x10, SEEK_CUR); // Skip world color
    }

    return pGroup;
}

CDependencyGroup* CUnsupportedFormatLoader::LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry)
{
    // RULE files can contain a reference to another RULE file, but has no other dependencies.
    uint32 Magic = rRULE.ReadLong();
    ASSERT(Magic == FOURCC('RULE'));

    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);
    rRULE.Seek(0x1, SEEK_CUR);

    // Version test
    uint32 IDOffset = rRULE.Tell();
    rRULE.Seek(0x4, SEEK_CUR);
    uint32 RuleSetCount = rRULE.ReadShort();
    EIDLength IDLength = (RuleSetCount > 0xFF ? e64Bit : e32Bit);
    rRULE.Seek(IDOffset, SEEK_SET);

    // Read rule ID
    CAssetID RuleID(rRULE, IDLength);
    pGroup->AddDependency(RuleID);
    return pGroup;
}
