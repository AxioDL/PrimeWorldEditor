#include "CWorldLoader.h"
#include "Core/Resource/CResCache.h"
#include "Core/Log.h"
#include <iostream>

CWorldLoader::CWorldLoader()
{
}

void CWorldLoader::LoadPrimeMLVL(CInputStream& MLVL)
{
    /**
     * This function loads MLVL files from Prime 1/2
     * Corruption isn't too different, but having to check for it on every file ID is obnoxious
     * We start immediately after the "version" value (0x8 in the file)
     */
    // Header
    if (mVersion < eCorruptionProto)
    {
        mpWorld->mpWorldName = gResCache.GetResource(MLVL.ReadLong(), "STRG");
        if (mVersion == eEchoes) mpWorld->mpDarkWorldName = gResCache.GetResource(MLVL.ReadLong(), "STRG");
        if (mVersion > ePrime)   mpWorld->mUnknown1 = MLVL.ReadLong();
        if (mVersion >= ePrime)  mpWorld->mpSaveWorld = gResCache.GetResource(MLVL.ReadLong(), "SAVW");
        mpWorld->mpDefaultSkybox = gResCache.GetResource(MLVL.ReadLong(), "CMDL");
    }

    else
    {
        mpWorld->mpWorldName = gResCache.GetResource(MLVL.ReadLongLong(), "STRG");
        MLVL.Seek(0x4, SEEK_CUR); // Skipping unknown value
        mpWorld->mpSaveWorld = gResCache.GetResource(MLVL.ReadLongLong(), "SAVW");
        mpWorld->mpDefaultSkybox = gResCache.GetResource(MLVL.ReadLongLong(), "CMDL");
    }

    // Memory relays - only in MP1
    if (mVersion == ePrime)
    {
        u32 NumMemoryRelays = MLVL.ReadLong();
        mpWorld->mMemoryRelays.reserve(NumMemoryRelays);

        for (u32 iMem = 0; iMem < NumMemoryRelays; iMem++)
        {
            CWorld::SMemoryRelay MemRelay;
            MemRelay.InstanceID = MLVL.ReadLong();
            MemRelay.TargetID = MLVL.ReadLong();
            MemRelay.Message = MLVL.ReadShort();
            MemRelay.Unknown = MLVL.ReadByte();
            mpWorld->mMemoryRelays.push_back(MemRelay);
        }
    }

    // Areas - here's the real meat of the file
    u32 NumAreas = MLVL.ReadLong();
    if (mVersion == ePrime) mpWorld->mUnknownAreas = MLVL.ReadLong();
    mpWorld->mAreas.resize(NumAreas);

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];

        if (mVersion < eCorruptionProto)
            pArea->pAreaName = gResCache.GetResource(MLVL.ReadLong(), "STRG");
        else
            pArea->pAreaName = gResCache.GetResource(MLVL.ReadLongLong(), "STRG");

        pArea->Transform = CTransform4f(MLVL);
        pArea->AetherBox = CAABox(MLVL);

        if (mVersion < eCorruptionProto)
        {
            pArea->FileID = MLVL.ReadLong() & 0xFFFFFFFF; // This is the MREA ID; not actually loading it for obvious reasons
            pArea->AreaID = MLVL.ReadLong() & 0xFFFFFFFF;
        }

        else
        {
            pArea->FileID = MLVL.ReadLongLong();
            pArea->AreaID = MLVL.ReadLongLong();
        }

        // Attached areas
        u32 NumAttachedAreas = MLVL.ReadLong();
        pArea->AttachedAreaIDs.reserve(NumAttachedAreas);
        for (u32 iAttached = 0; iAttached < NumAttachedAreas; iAttached++)
            pArea->AttachedAreaIDs.push_back( MLVL.ReadShort() );

        if (mVersion < eCorruptionProto)
            MLVL.Seek(0x4, SEEK_CUR); // Skipping unknown value (always 0)

        // Depedencies
        if (mVersion < eCorruptionProto)
        {
            u32 NumDependencies = MLVL.ReadLong();
            pArea->Dependencies.reserve(NumDependencies);

            for (u32 iDep = 0; iDep < NumDependencies; iDep++)
            {
                SDependency Dependency;
                Dependency.ResID = MLVL.ReadLong() & 0xFFFFFFFF;
                Dependency.ResType = MLVL.ReadLong();
                pArea->Dependencies.push_back(Dependency);
            }

            /**
             * Dependency offsets - indicates an offset into the dependency list where each layer's dependencies start
             * The count is the layer count + 1 because the last offset is for common dependencies, like terrain textures
             */
            u32 NumDependencyOffsets = MLVL.ReadLong();
            pArea->Layers.resize(NumDependencyOffsets - 1);

            for (u32 iOff = 0; iOff < NumDependencyOffsets; iOff++)
            {
                u32 *Target;
                if (iOff == NumDependencyOffsets - 1) Target = &pArea->CommonDependenciesStart;
                else Target = &pArea->Layers[iOff].LayerDependenciesStart;

                *Target = MLVL.ReadLong();
            }
        }

        // Docks
        u32 NumDocks = MLVL.ReadLong();
        pArea->Docks.resize(NumDocks);

        for (u32 iDock = 0; iDock < NumDocks; iDock++)
        {
            u32 NumConnectingDocks = MLVL.ReadLong();

            CWorld::SArea::SDock* pDock = &pArea->Docks[iDock];
            pDock->ConnectingDocks.reserve(NumConnectingDocks);

            for (u32 iConnect = 0; iConnect < NumConnectingDocks; iConnect++)
            {
                CWorld::SArea::SDock::SConnectingDock ConnectingDock;
                ConnectingDock.AreaIndex = MLVL.ReadLong();
                ConnectingDock.DockIndex = MLVL.ReadLong();
                pDock->ConnectingDocks.push_back(ConnectingDock);
            }

            u32 NumCoordinates = MLVL.ReadLong();
            if (NumCoordinates != 4) std::cout << "\rError: Dock coordinate count not 4\n";

            for (u32 iCoord = 0; iCoord < NumCoordinates; iCoord++)
                pDock->DockCoordinates[iCoord] = CVector3f(MLVL);
        }

        // Rels
        if (mVersion == eEchoes)
        {
            u32 NumRels = MLVL.ReadLong();
            pArea->RelFilenames.resize(NumRels);

            for (u32 iRel = 0; iRel < NumRels; iRel++)
                pArea->RelFilenames[iRel] = MLVL.ReadString();

            u32 NumRelOffsets = MLVL.ReadLong(); // Don't know what these offsets correspond to
            pArea->RelOffsets.resize(NumRelOffsets);

            for (u32 iOff = 0; iOff < NumRelOffsets; iOff++)
                pArea->RelOffsets[iOff] = MLVL.ReadLong();
        }

        // Footer
        if (mVersion >= eEchoes)
            pArea->InternalName = MLVL.ReadString();
    }

    // MapWorld
    if (mVersion < eCorruptionProto)
        mpWorld->mpMapWorld = gResCache.GetResource(MLVL.ReadLong(), "MAPW");
    else
        mpWorld->mpMapWorld = gResCache.GetResource(MLVL.ReadLongLong(), "MAPW");
    MLVL.Seek(0x5, SEEK_CUR); // Unknown values which are always 0

    // AudioGrps
    if (mVersion == ePrime)
    {
        u32 NumAudioGrps = MLVL.ReadLong();
        mpWorld->mAudioGrps.reserve(NumAudioGrps);

        for (u32 iGrp = 0; iGrp < NumAudioGrps; iGrp++)
        {
            CWorld::SAudioGrp AudioGrp;
            AudioGrp.Unknown = MLVL.ReadLong();
            AudioGrp.ResID = MLVL.ReadLong() & 0xFFFFFFFF;
            mpWorld->mAudioGrps.push_back(AudioGrp);
        }

        MLVL.Seek(0x1, SEEK_CUR); // Unknown values which are always 0
    }

    // Layer flags
    MLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = MLVL.ReadLong();
        if (NumLayers != pArea->Layers.size()) pArea->Layers.resize(NumLayers);

        u64 LayerFlags = MLVL.ReadLongLong();
        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].EnabledByDefault = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    MLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = pArea->Layers.size();

        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = MLVL.ReadString();
    }

    // Last part of the file is layer name offsets, but we don't need it
    // todo: Layer ID support for MP3
}

void CWorldLoader::LoadReturnsMLVL(CInputStream& MLVL)
{
    mpWorld->mpWorldName = gResCache.GetResource(MLVL.ReadLongLong(), "STRG");

    bool Check = (MLVL.ReadByte() != 0);
    if (Check)
    {
        MLVL.ReadString();
        MLVL.Seek(0x10, SEEK_CUR);
    }

    mpWorld->mpSaveWorld = gResCache.GetResource(MLVL.ReadLongLong(), "SAVW");
    mpWorld->mpDefaultSkybox = gResCache.GetResource(MLVL.ReadLongLong(), "CMDL");

    // Areas
    u32 NumAreas = MLVL.ReadLong();
    mpWorld->mAreas.resize(NumAreas);

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];

        pArea->pAreaName = gResCache.GetResource(MLVL.ReadLongLong(), "STRG");
        pArea->Transform = CTransform4f(MLVL);
        pArea->AetherBox = CAABox(MLVL);
        pArea->FileID = MLVL.ReadLongLong();
        pArea->AreaID = MLVL.ReadLongLong();

        MLVL.Seek(0x4, SEEK_CUR);
        pArea->InternalName = MLVL.ReadString();
    }

    // Layer flags
    MLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = MLVL.ReadLong();
        pArea->Layers.resize(NumLayers);

        u64 LayerFlags = MLVL.ReadLongLong();
        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].EnabledByDefault = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    MLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = pArea->Layers.size();

        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = MLVL.ReadString();
    }

    // Last part of the file is layer name offsets, but we don't need it
    // todo: Layer ID support
}

CWorld* CWorldLoader::LoadMLVL(CInputStream& MLVL)
{
    if (!MLVL.IsValid()) return nullptr;
    Log::Write("Loading " + MLVL.GetSourceString());

    u32 Magic = MLVL.ReadLong();
    if (Magic != 0xDEAFBABE)
    {
        Log::FileError(MLVL.GetSourceString(), "Invalid MLVL magic: " + TString::HexString(Magic));
        return nullptr;
    }

    u32 FileVersion = MLVL.ReadLong();
    EGame Version = GetFormatVersion(FileVersion);
    if (Version == eUnknownVersion)
    {
        Log::FileError(MLVL.GetSourceString(), "Unsupported MLVL version: " + TString::HexString(FileVersion));
        return nullptr;
    }

    // Filestream is valid, magic+version are valid; everything seems good!
    CWorldLoader Loader;
    Loader.mpWorld = new CWorld();
    Loader.mpWorld->mWorldVersion = Version;
    Loader.mVersion = Version;

    if (Version != eReturns)
        Loader.LoadPrimeMLVL(MLVL);
    else
        Loader.LoadReturnsMLVL(MLVL);

    return Loader.mpWorld;
}

EGame CWorldLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
        case 0xD: return ePrimeDemo;
        case 0x11: return ePrime;
        case 0x14: return eEchoesDemo;
        case 0x17: return eEchoes;
        case 0x19: return eCorruption;
        case 0x1B: return eReturns;
        default: return eUnknownVersion;
    }
}
