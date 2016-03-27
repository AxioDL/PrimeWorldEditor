#include "CWorldLoader.h"
#include "Core/Resource/CResCache.h"
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
    if (mVersion < eCorruptionProto)
    {
        mpWorld->mpWorldName = gResCache.GetResource(rMLVL.ReadLong(), "STRG");
        if (mVersion == eEchoes) mpWorld->mpDarkWorldName = gResCache.GetResource(rMLVL.ReadLong(), "STRG");
        if (mVersion >= eEchoes) mpWorld->mUnknown1 = rMLVL.ReadLong();
        if (mVersion >= ePrime) mpWorld->mpSaveWorld = gResCache.GetResource(rMLVL.ReadLong(), "SAVW");
        mpWorld->mpDefaultSkybox = gResCache.GetResource(rMLVL.ReadLong(), "CMDL");
    }

    else
    {
        mpWorld->mpWorldName = gResCache.GetResource(rMLVL.ReadLongLong(), "STRG");
        rMLVL.Seek(0x4, SEEK_CUR); // Skipping unknown value
        mpWorld->mpSaveWorld = gResCache.GetResource(rMLVL.ReadLongLong(), "SAVW");
        mpWorld->mpDefaultSkybox = gResCache.GetResource(rMLVL.ReadLongLong(), "CMDL");
    }

    // Memory relays - only in MP1
    if (mVersion == ePrime)
    {
        u32 NumMemoryRelays = rMLVL.ReadLong();
        mpWorld->mMemoryRelays.reserve(NumMemoryRelays);

        for (u32 iMem = 0; iMem < NumMemoryRelays; iMem++)
        {
            CWorld::SMemoryRelay MemRelay;
            MemRelay.InstanceID = rMLVL.ReadLong();
            MemRelay.TargetID = rMLVL.ReadLong();
            MemRelay.Message = rMLVL.ReadShort();
            MemRelay.Unknown = rMLVL.ReadByte();
            mpWorld->mMemoryRelays.push_back(MemRelay);
        }
    }

    // Areas - here's the real meat of the file
    u32 NumAreas = rMLVL.ReadLong();
    if (mVersion == ePrime) mpWorld->mUnknownAreas = rMLVL.ReadLong();
    mpWorld->mAreas.resize(NumAreas);

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];

        if (mVersion < eCorruptionProto)
            pArea->pAreaName = gResCache.GetResource(rMLVL.ReadLong(), "STRG");
        else
            pArea->pAreaName = gResCache.GetResource(rMLVL.ReadLongLong(), "STRG");

        pArea->Transform = CTransform4f(rMLVL);
        pArea->AetherBox = CAABox(rMLVL);

        if (mVersion < eCorruptionProto)
        {
            pArea->FileID = rMLVL.ReadLong() & 0xFFFFFFFF; // This is the MREA ID; not actually loading it for obvious reasons
            pArea->AreaID = rMLVL.ReadLong() & 0xFFFFFFFF;
        }

        else
        {
            pArea->FileID = rMLVL.ReadLongLong();
            pArea->AreaID = rMLVL.ReadLongLong();
        }

        // Attached areas
        u32 NumAttachedAreas = rMLVL.ReadLong();
        pArea->AttachedAreaIDs.reserve(NumAttachedAreas);
        for (u32 iAttached = 0; iAttached < NumAttachedAreas; iAttached++)
            pArea->AttachedAreaIDs.push_back( rMLVL.ReadShort() );

        if (mVersion < eCorruptionProto)
            rMLVL.Seek(0x4, SEEK_CUR); // Skipping unknown value (always 0)

        // Depedencies
        if (mVersion < eCorruptionProto)
        {
            u32 NumDependencies = rMLVL.ReadLong();
            pArea->Dependencies.reserve(NumDependencies);

            for (u32 iDep = 0; iDep < NumDependencies; iDep++)
            {
                SDependency Dependency;
                Dependency.ResID = rMLVL.ReadLong() & 0xFFFFFFFF;
                Dependency.ResType = rMLVL.ReadLong();
                pArea->Dependencies.push_back(Dependency);
            }

            /**
             * Dependency offsets - indicates an offset into the dependency list where each layer's dependencies start
             * The count is the layer count + 1 because the last offset is for common dependencies, like terrain textures
             */
            u32 NumDependencyOffsets = rMLVL.ReadLong();
            pArea->Layers.resize(NumDependencyOffsets - 1);

            for (u32 iOff = 0; iOff < NumDependencyOffsets; iOff++)
            {
                u32 *pTarget;
                if (iOff == NumDependencyOffsets - 1) pTarget = &pArea->CommonDependenciesStart;
                else pTarget = &pArea->Layers[iOff].LayerDependenciesStart;

                *pTarget = rMLVL.ReadLong();
            }
        }

        // Docks
        u32 NumDocks = rMLVL.ReadLong();
        pArea->Docks.resize(NumDocks);

        for (u32 iDock = 0; iDock < NumDocks; iDock++)
        {
            u32 NumConnectingDocks = rMLVL.ReadLong();

            CWorld::SArea::SDock* pDock = &pArea->Docks[iDock];
            pDock->ConnectingDocks.reserve(NumConnectingDocks);

            for (u32 iConnect = 0; iConnect < NumConnectingDocks; iConnect++)
            {
                CWorld::SArea::SDock::SConnectingDock ConnectingDock;
                ConnectingDock.AreaIndex = rMLVL.ReadLong();
                ConnectingDock.DockIndex = rMLVL.ReadLong();
                pDock->ConnectingDocks.push_back(ConnectingDock);
            }

            u32 NumCoordinates = rMLVL.ReadLong();
            if (NumCoordinates != 4) Log::Error("Dock coordinate count not 4");

            for (u32 iCoord = 0; iCoord < NumCoordinates; iCoord++)
                pDock->DockCoordinates[iCoord] = CVector3f(rMLVL);
        }

        // Rels
        if ( (mVersion == eEchoesDemo) || (mVersion == eEchoes) )
        {
            u32 NumRels = rMLVL.ReadLong();
            pArea->RelFilenames.resize(NumRels);

            for (u32 iRel = 0; iRel < NumRels; iRel++)
                pArea->RelFilenames[iRel] = rMLVL.ReadString();

            if (mVersion == eEchoes)
            {
                u32 NumRelOffsets = rMLVL.ReadLong(); // Don't know what these offsets correspond to
                pArea->RelOffsets.resize(NumRelOffsets);

                for (u32 iOff = 0; iOff < NumRelOffsets; iOff++)
                    pArea->RelOffsets[iOff] = rMLVL.ReadLong();
            }
        }

        // Footer
        if (mVersion >= eEchoesDemo)
            pArea->InternalName = rMLVL.ReadString();
    }

    // MapWorld
    if (mVersion < eCorruptionProto)
        mpWorld->mpMapWorld = gResCache.GetResource(rMLVL.ReadLong(), "MAPW");
    else
        mpWorld->mpMapWorld = gResCache.GetResource(rMLVL.ReadLongLong(), "MAPW");
    rMLVL.Seek(0x5, SEEK_CUR); // Unknown values which are always 0

    // AudioGrps
    if (mVersion == ePrime)
    {
        u32 NumAudioGrps = rMLVL.ReadLong();
        mpWorld->mAudioGrps.reserve(NumAudioGrps);

        for (u32 iGrp = 0; iGrp < NumAudioGrps; iGrp++)
        {
            CWorld::SAudioGrp AudioGrp;
            AudioGrp.Unknown = rMLVL.ReadLong();
            AudioGrp.ResID = rMLVL.ReadLong() & 0xFFFFFFFF;
            mpWorld->mAudioGrps.push_back(AudioGrp);
        }

        rMLVL.Seek(0x1, SEEK_CUR); // Unknown values which are always 0
    }

    // Layer flags
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = rMLVL.ReadLong();
        if (NumLayers != pArea->Layers.size()) pArea->Layers.resize(NumLayers);

        u64 LayerFlags = rMLVL.ReadLongLong();
        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].EnabledByDefault = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = pArea->Layers.size();

        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = rMLVL.ReadString();
    }

    // Last part of the file is layer name offsets, but we don't need it
    // todo: Layer ID support for MP3
}

void CWorldLoader::LoadReturnsMLVL(IInputStream& rMLVL)
{
    mpWorld->mpWorldName = gResCache.GetResource(rMLVL.ReadLongLong(), "STRG");

    bool Check = (rMLVL.ReadByte() != 0);
    if (Check)
    {
        rMLVL.ReadString();
        rMLVL.Seek(0x10, SEEK_CUR);
    }

    mpWorld->mpSaveWorld = gResCache.GetResource(rMLVL.ReadLongLong(), "SAVW");
    mpWorld->mpDefaultSkybox = gResCache.GetResource(rMLVL.ReadLongLong(), "CMDL");

    // Areas
    u32 NumAreas = rMLVL.ReadLong();
    mpWorld->mAreas.resize(NumAreas);

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        // Area header
        CWorld::SArea *pArea = &mpWorld->mAreas[iArea];

        pArea->pAreaName = gResCache.GetResource(rMLVL.ReadLongLong(), "STRG");
        pArea->Transform = CTransform4f(rMLVL);
        pArea->AetherBox = CAABox(rMLVL);
        pArea->FileID = rMLVL.ReadLongLong();
        pArea->AreaID = rMLVL.ReadLongLong();

        rMLVL.Seek(0x4, SEEK_CUR);
        pArea->InternalName = rMLVL.ReadString();
    }

    // Layer flags
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant area count

    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = rMLVL.ReadLong();
        pArea->Layers.resize(NumLayers);

        u64 LayerFlags = rMLVL.ReadLongLong();
        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].EnabledByDefault = (((LayerFlags >> iLayer) & 0x1) == 1);
    }

    // Layer names
    rMLVL.Seek(0x4, SEEK_CUR); // Skipping redundant layer count
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CWorld::SArea* pArea = &mpWorld->mAreas[iArea];
        u32 NumLayers = pArea->Layers.size();

        for (u32 iLayer = 0; iLayer < NumLayers; iLayer++)
            pArea->Layers[iLayer].LayerName = rMLVL.ReadString();
    }

    // Last part of the file is layer name offsets, but we don't need it
    // todo: Layer ID support
}

CWorld* CWorldLoader::LoadMLVL(IInputStream& rMLVL)
{
    if (!rMLVL.IsValid()) return nullptr;

    u32 Magic = rMLVL.ReadLong();
    if (Magic != 0xDEAFBABE)
    {
        Log::FileError(rMLVL.GetSourceString(), "Invalid MLVL magic: " + TString::HexString(Magic));
        return nullptr;
    }

    u32 FileVersion = rMLVL.ReadLong();
    EGame Version = GetFormatVersion(FileVersion);
    if (Version == eUnknownVersion)
    {
        Log::FileError(rMLVL.GetSourceString(), "Unsupported MLVL version: " + TString::HexString(FileVersion, 2));
        return nullptr;
    }

    // Filestream is valid, magic+version are valid; everything seems good!
    CWorldLoader Loader;
    Loader.mpWorld = new CWorld();
    Loader.mpWorld->mWorldVersion = Version;
    Loader.mVersion = Version;

    if (Version != eReturns)
        Loader.LoadPrimeMLVL(rMLVL);
    else
        Loader.LoadReturnsMLVL(rMLVL);

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
