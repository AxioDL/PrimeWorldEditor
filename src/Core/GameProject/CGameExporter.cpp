#include "CGameExporter.h"
#include "CGameInfo.h"
#include "CResourceIterator.h"
#include "CResourceStore.h"
#include "Core/CompressionUtil.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <Common/Macros.h>
#include <Common/CScopedTimer.h>
#include <Common/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/Serialization/CXMLWriter.h>

#include <nod/nod.hpp>
#include <tinyxml2.h>

#define LOAD_PAKS 1
#define SAVE_PACKAGE_DEFINITIONS 1
#define USE_ASSET_NAME_MAP 1
#define EXPORT_COOKED 1

CGameExporter::CGameExporter(EDiscType DiscType, EGame Game, bool FrontEnd, ERegion Region, const TString& rkGameName, const TString& rkGameID, float BuildVersion)
    : mGame(Game)
    , mRegion(Region)
    , mGameName(rkGameName)
    , mGameID(rkGameID)
    , mBuildVersion(BuildVersion)
    , mDiscType(DiscType)
    , mFrontEnd(FrontEnd)
    , mpProgress(nullptr)
{
    ASSERT(mGame != EGame::Invalid);
    ASSERT(mRegion != ERegion::Unknown);
}

bool CGameExporter::Export(nod::DiscBase *pDisc, const TString& rkOutputDir, CAssetNameMap *pNameMap, CGameInfo *pGameInfo, IProgressNotifier *pProgress)
{
    SCOPED_TIMER(ExportGame);

    mpDisc = pDisc;
    mpNameMap = pNameMap;
    mpGameInfo = pGameInfo;

    mExportDir = FileUtil::MakeAbsolute(rkOutputDir);
    mDiscDir = "Disc/";
    mWorldsDirName = "Worlds/";

    // Export directory must be empty!
    if (FileUtil::Exists(mExportDir) && !FileUtil::IsEmpty(mExportDir))
        return false;

    FileUtil::MakeDirectory(mExportDir);

    // Init progress
    mpProgress = pProgress;
    mpProgress->SetNumTasks(eES_NumSteps);

    // Extract disc
    if (!ExtractDiscData())
        return false;

    // Create project
    mpProject = CGameProject::CreateProjectForExport(
                mExportDir,
                mGame,
                mRegion,
                mGameID,
                mBuildVersion);

    mpProject->SetProjectName(mGameName);
    mpStore = mpProject->ResourceStore();
    mResourcesDir = mpStore->ResourcesDir();

    CResourceStore *pOldStore = gpResourceStore;
    gpResourceStore = mpStore;

    // Export cooked data
    LoadPaks();
    ExportCookedResources();

    // Export editor data
    if (!mpProgress->ShouldCancel())
    {
        mpProject->AudioManager()->LoadAssets();
        ExportResourceEditorData();
    }

    // Export finished!
    mProjectPath = mpProject->ProjectPath();
    delete mpProject;
    if (pOldStore) gpResourceStore = pOldStore;
    return !mpProgress->ShouldCancel();
}

void CGameExporter::LoadResource(const CAssetID& rkID, std::vector<uint8>& rBuffer)
{
    SResourceInstance *pInst = FindResourceInstance(rkID);
    if (pInst) LoadResource(*pInst, rBuffer);
}

bool CGameExporter::ShouldExportDiscNode(const nod::Node *pkNode, bool IsInRoot)
{
    if (IsInRoot && mDiscType != EDiscType::Normal)
    {
        // Directories - exclude the filesystem for other games
        if (pkNode->getKind() == nod::Node::Kind::Directory)
        {
            // Frontend is always included; this is for compatibility with Dolphin
            if (pkNode->getName() == "fe")
                return true;

            else if (mFrontEnd)
                return false;

            switch (mGame)
            {
            case EGame::Prime:
                return ( (mDiscType == EDiscType::WiiDeAsobu && pkNode->getName() == "MP1JPN") ||
                         (mDiscType == EDiscType::Trilogy && pkNode->getName() == "MP1") );

            case EGame::Echoes:
                return ( (mDiscType == EDiscType::WiiDeAsobu && pkNode->getName() == "MP2JPN") ||
                         (mDiscType == EDiscType::Trilogy && pkNode->getName() == "MP2") );

            case EGame::Corruption:
                return (mDiscType == EDiscType::Trilogy && pkNode->getName() == "MP3");

            default:
                return false;
            }
        }

        // Files - exclude the DOLs for other games
        else
        {
            // Again - always include frontend. Always include opening.bnr as well.
            if (pkNode->getName() == "rs5fe_p.dol" || pkNode->getName() == "opening.bnr")
                return true;

            else if (mFrontEnd)
                return false;

            switch (mGame)
            {
            case EGame::Prime:
                return ( (mDiscType == EDiscType::WiiDeAsobu && pkNode->getName() == "rs5mp1jpn_p.dol") ||
                         (mDiscType == EDiscType::Trilogy && pkNode->getName() == "rs5mp1_p.dol") );

            case EGame::Echoes:
                return ( (mDiscType == EDiscType::WiiDeAsobu && pkNode->getName() == "rs5mp2jpn_p.dol") ||
                         (mDiscType == EDiscType::Trilogy && pkNode->getName() == "rs5mp2_p.dol") );

            case EGame::Corruption:
                return (mDiscType == EDiscType::Trilogy && pkNode->getName() == "rs5mp3_p.dol");

            default:
                return false;
            }
        }
    }

    return true;
}

// ************ PROTECTED ************
bool CGameExporter::ExtractDiscData()
{
    // todo: handle dol, apploader, multiple partitions, wii ticket blob
    SCOPED_TIMER(ExtractDiscData);

    // Init progress
    mpProgress->SetTask(eES_ExtractDisc, "Extracting disc files");

    // Create Disc output folder
    TString AbsDiscDir = mExportDir + mDiscDir;
    bool IsWii = (mBuildVersion >= 3.f);
    if (IsWii) AbsDiscDir += "DATA/";
    FileUtil::MakeDirectory(AbsDiscDir);

    // Extract disc filesystem
    nod::IPartition *pDataPartition = mpDisc->getDataPartition();
    nod::ExtractionContext Context;
    Context.force = false;
    Context.progressCB = [&](const std::string_view rkDesc, float ProgressPercent) {
        mpProgress->Report((int) (ProgressPercent * 10000), 10000, rkDesc.data());
    };

    TString FilesDir = AbsDiscDir + "files/";
    FileUtil::MakeDirectory(FilesDir);

    bool Success = ExtractDiscNodeRecursive(&pDataPartition->getFSTRoot(), FilesDir, true, Context);
    if (!Success) return false;

    if (!mpProgress->ShouldCancel())
    {
        Context.progressCB = nullptr;

        if (IsWii)
        {
            // Extract crypto files
            if (!pDataPartition->extractCryptoFiles(ToWChar(AbsDiscDir), Context))
                return false;

            // Extract disc header files
            if (!mpDisc->extractDiscHeaderFiles(ToWChar(AbsDiscDir), Context))
                return false;
        }

        // Extract system files
        if (!pDataPartition->extractSysFiles(ToWChar(AbsDiscDir), Context))
            return false;

        return true;
    }
    else
        return false;
}

bool CGameExporter::ExtractDiscNodeRecursive(const nod::Node *pkNode, const TString& rkDir, bool RootNode, const nod::ExtractionContext& rkContext)
{
    for (nod::Node::DirectoryIterator Iter = pkNode->begin(); Iter != pkNode->end(); ++Iter)
    {
        if (!ShouldExportDiscNode(&*Iter, RootNode))
            continue;

        if (Iter->getKind() == nod::Node::Kind::File)
        {
            TString FilePath = rkDir + Iter->getName().data();
            bool Success = Iter->extractToDirectory(ToWChar(rkDir), rkContext);
            if (!Success) return false;

            if (FilePath.GetFileExtension().CaseInsensitiveCompare("pak"))
            {
                // For multi-game Wii discs, don't track packages for frontend unless we're exporting frontend
                if (mDiscType == EDiscType::Normal || mFrontEnd || pkNode->getName() != "fe")
                    mPaks.push_back(FilePath);
            }
        }

        else
        {
            TString Subdir = rkDir + Iter->getName().data() + "/";
            bool Success = FileUtil::MakeDirectory(Subdir);
            if (!Success) return false;

            Success = ExtractDiscNodeRecursive(&*Iter, Subdir, false, rkContext);
            if (!Success) return false;
        }
    }

    return true;
}

// ************ RESOURCE LOADING ************
void CGameExporter::LoadPaks()
{
#if LOAD_PAKS
    SCOPED_TIMER(LoadPaks);

    mPaks.sort([](const TString& rkLeft, const TString& rkRight) -> bool {
        return rkLeft.ToUpper() < rkRight.ToUpper();
    });

    for (auto It = mPaks.begin(); It != mPaks.end(); It++)
    {
        TString PakPath = *It;
        CFileInStream Pak(PakPath, EEndian::BigEndian);

        if (!Pak.IsValid())
        {
            errorf("Couldn't open pak: %s", *PakPath);
            continue;
        }

        TString RelPakPath = FileUtil::MakeRelative(PakPath.GetFileDirectory(), mpProject->DiscFilesystemRoot(false));
        CPackage *pPackage = new CPackage(mpProject, PakPath.GetFileName(false), RelPakPath);

        // MP1-MP3Proto
        if (mGame < EGame::Corruption)
        {
            uint32 PakVersion = Pak.ReadLong();
            Pak.Seek(0x4, SEEK_CUR);
            ASSERT(PakVersion == 0x00030005);

            // Echoes demo disc has a pak that ends right here.
            if (!Pak.EoF())
            {
                uint32 NumNamedResources = Pak.ReadLong();
                ASSERT(NumNamedResources > 0);

                for (uint32 iName = 0; iName < NumNamedResources; iName++)
                {
                    CFourCC ResType = Pak.ReadLong();
                    CAssetID ResID(Pak, mGame);
                    uint32 NameLen = Pak.ReadLong();
                    TString Name = Pak.ReadString(NameLen);
                    pPackage->AddResource(Name, ResID, ResType);
                }

                uint32 NumResources = Pak.ReadLong();

                // Keep track of which areas have duplicate resources
                std::set<CAssetID> PakResourceSet;
                bool AreaHasDuplicates = true; // Default to true so that first area is always considered as having duplicates

                for (uint32 iRes = 0; iRes < NumResources; iRes++)
                {
                    bool Compressed = (Pak.ReadLong() == 1);
                    CFourCC ResType = Pak.ReadLong();
                    CAssetID ResID(Pak, mGame);
                    uint32 ResSize = Pak.ReadLong();
                    uint32 ResOffset = Pak.ReadLong();

                    if (mResourceMap.find(ResID) == mResourceMap.end())
                        mResourceMap[ResID] = SResourceInstance { PakPath, ResID, ResType, ResOffset, ResSize, Compressed, false };

                    // Check for duplicate resources
                    if (ResType == "MREA")
                    {
                        mAreaDuplicateMap[ResID] = AreaHasDuplicates;
                        AreaHasDuplicates = false;
                    }

                    else if (!AreaHasDuplicates && PakResourceSet.find(ResID) != PakResourceSet.end())
                        AreaHasDuplicates = true;

                    else
                        PakResourceSet.insert(ResID);
                }
            }
        }

        // MP3 + DKCR
        else
        {
            uint32 PakVersion = Pak.ReadLong();
            uint32 PakHeaderLen = Pak.ReadLong();
            Pak.Seek(PakHeaderLen - 0x8, SEEK_CUR);
            ASSERT(PakVersion == 2);

            struct SPakSection {
                CFourCC Type; uint32 Size;
            };
            std::vector<SPakSection> PakSections;

            uint32 NumPakSections = Pak.ReadLong();
            ASSERT(NumPakSections == 3);

            for (uint32 iSec = 0; iSec < NumPakSections; iSec++)
            {
                CFourCC Type = Pak.ReadLong();
                uint32 Size = Pak.ReadLong();
                PakSections.push_back(SPakSection { Type, Size });
            }
            Pak.SeekToBoundary(64);

            for (uint32 iSec = 0; iSec < NumPakSections; iSec++)
            {
                uint32 Next = Pak.Tell() + PakSections[iSec].Size;

                // Named Resources
                if (PakSections[iSec].Type == "STRG")
                {
                    uint32 NumNamedResources = Pak.ReadLong();

                    for (uint32 iName = 0; iName < NumNamedResources; iName++)
                    {
                        TString Name = Pak.ReadString();
                        CFourCC ResType = Pak.ReadLong();
                        CAssetID ResID(Pak, mGame);
                        pPackage->AddResource(Name, ResID, ResType);
                    }
                }

                else if (PakSections[iSec].Type == "RSHD")
                {
                    ASSERT(PakSections[iSec + 1].Type == "DATA");
                    uint32 DataStart = Next;
                    uint32 NumResources = Pak.ReadLong();

                    // Keep track of which areas have duplicate resources
                    std::set<CAssetID> PakResourceSet;
                    bool AreaHasDuplicates = true; // Default to true so that first area is always considered as having duplicates

                    for (uint32 iRes = 0; iRes < NumResources; iRes++)
                    {
                        bool Compressed = (Pak.ReadLong() == 1);
                        CFourCC Type = Pak.ReadLong();
                        CAssetID ResID(Pak, mGame);
                        uint32 Size = Pak.ReadLong();
                        uint32 Offset = DataStart + Pak.ReadLong();

                        if (mResourceMap.find(ResID) == mResourceMap.end())
                            mResourceMap[ResID] = SResourceInstance { PakPath, ResID, Type, Offset, Size, Compressed, false };

                        // Check for duplicate resources (unnecessary for DKCR)
                        if (mGame != EGame::DKCReturns)
                        {
                            if (Type == "MREA")
                            {
                                mAreaDuplicateMap[ResID] = AreaHasDuplicates;
                                AreaHasDuplicates = false;
                            }

                            else if (!AreaHasDuplicates && PakResourceSet.find(ResID) != PakResourceSet.end())
                                AreaHasDuplicates = true;

                            else
                                PakResourceSet.insert(ResID);
                        }
                    }
                }

                Pak.Seek(Next, SEEK_SET);
            }
        }

        // Add package to project and save
        mpProject->AddPackage(pPackage);
#if SAVE_PACKAGE_DEFINITIONS
        bool SaveSuccess = pPackage->Save();
        ASSERT(SaveSuccess);
#endif
    }
#endif
}

void CGameExporter::LoadResource(const SResourceInstance& rkResource, std::vector<uint8>& rBuffer)
{
    CFileInStream Pak(rkResource.PakFile, EEndian::BigEndian);

    if (Pak.IsValid())
    {
        Pak.Seek(rkResource.PakOffset, SEEK_SET);

        // Handle compression
        if (rkResource.Compressed)
        {
            bool ZlibCompressed = (mGame <= EGame::EchoesDemo || mGame == EGame::DKCReturns);

            if (mGame <= EGame::CorruptionProto)
            {
                std::vector<uint8> CompressedData(rkResource.PakSize);

                uint32 UncompressedSize = Pak.ReadLong();
                rBuffer.resize(UncompressedSize);
                Pak.ReadBytes(CompressedData.data(), CompressedData.size());

                if (ZlibCompressed)
                {
                    uint32 TotalOut;
                    CompressionUtil::DecompressZlib(CompressedData.data(), CompressedData.size(), rBuffer.data(), rBuffer.size(), TotalOut);
                }
                else
                {
                    CompressionUtil::DecompressSegmentedData(CompressedData.data(), CompressedData.size(), rBuffer.data(), rBuffer.size());
                }
            }

            else
            {
                CFourCC Magic = Pak.ReadLong();
                ASSERT(Magic == "CMPD");

                uint32 NumBlocks = Pak.ReadLong();

                struct SCompressedBlock {
                    uint32 CompressedSize; uint32 UncompressedSize;
                };
                std::vector<SCompressedBlock> CompressedBlocks;

                uint32 TotalUncompressedSize = 0;
                for (uint32 iBlock = 0; iBlock < NumBlocks; iBlock++)
                {
                    uint32 CompressedSize = (Pak.ReadLong() & 0x00FFFFFF);
                    uint32 UncompressedSize = Pak.ReadLong();

                    TotalUncompressedSize += UncompressedSize;
                    CompressedBlocks.push_back( SCompressedBlock { CompressedSize, UncompressedSize } );
                }

                rBuffer.resize(TotalUncompressedSize);
                uint32 Offset = 0;

                for (uint32 iBlock = 0; iBlock < NumBlocks; iBlock++)
                {
                    uint32 CompressedSize = CompressedBlocks[iBlock].CompressedSize;
                    uint32 UncompressedSize = CompressedBlocks[iBlock].UncompressedSize;

                    // Block is compressed
                    if (CompressedSize != UncompressedSize)
                    {
                        std::vector<uint8> CompressedData(CompressedBlocks[iBlock].CompressedSize);
                        Pak.ReadBytes(CompressedData.data(), CompressedData.size());

                        if (ZlibCompressed)
                        {
                            uint32 TotalOut;
                            CompressionUtil::DecompressZlib(CompressedData.data(), CompressedData.size(), rBuffer.data() + Offset, UncompressedSize, TotalOut);
                        }
                        else
                        {
                            CompressionUtil::DecompressSegmentedData(CompressedData.data(), CompressedData.size(), rBuffer.data() + Offset, UncompressedSize);
                        }
                    }
                    // Block is uncompressed
                    else
                        Pak.ReadBytes(rBuffer.data() + Offset, UncompressedSize);

                    Offset += UncompressedSize;
                }
            }
        }

        // Handle uncompressed
        else
        {
            rBuffer.resize(rkResource.PakSize);
            Pak.ReadBytes(rBuffer.data(), rBuffer.size());
        }
    }
}

void CGameExporter::ExportCookedResources()
{
    SCOPED_TIMER(ExportCookedResources);
    FileUtil::MakeDirectory(mResourcesDir);

    mpProgress->SetTask(eES_ExportCooked, "Unpacking cooked assets");
    int ResIndex = 0;

    for (auto It = mResourceMap.begin(); It != mResourceMap.end() && !mpProgress->ShouldCancel(); It++, ResIndex++)
    {
        SResourceInstance& rRes = It->second;

        // Update progress
        if ((ResIndex & 0x3) == 0)
            mpProgress->Report(ResIndex, mResourceMap.size(), TString::Format("Unpacking asset %d/%d", ResIndex, mResourceMap.size()) );

        // Export resource
        ExportResource(rRes);
    }
}

void CGameExporter::ExportResourceEditorData()
{
    {
        // Save raw versions of resources + resource cache data files
        // Note this has to be done after all cooked resources are exported
        // because we have to load the resource to build its dependency tree and
        // some resources will fail to load if their dependencies don't exist
        SCOPED_TIMER(SaveRawResources);
        mpProgress->SetTask(eES_GenerateRaw, "Generating editor data");
        int ResIndex = 0;

        // todo: we're wasting a ton of time loading the same resources over and over because most resources automatically
        // load all their dependencies and then we just clear it out from memory even though we'll need it again later. we
        // should really be doing this by dependency order instead of by ID order.
        for (CResourceIterator It(mpStore); It && !mpProgress->ShouldCancel(); ++It, ++ResIndex)
        {
            // Update progress
            if ((ResIndex & 0x3) == 0 || It->ResourceType() == EResourceType::Area)
                mpProgress->Report(ResIndex, mpStore->NumTotalResources(), TString::Format("Processing asset %d/%d: %s",
                    ResIndex, mpStore->NumTotalResources(), *It->CookedAssetPath(true).GetFileName()) );

            // Worlds need some info we can only get from the pak at export time; namely, which areas can
            // have duplicates, as well as the world's internal name.
            if (It->ResourceType() == EResourceType::World)
            {
                CWorld *pWorld = (CWorld*) It->Load();

                // Set area duplicate flags
                for (uint32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                {
                    CAssetID AreaID = pWorld->AreaResourceID(iArea);
                    auto Find = mAreaDuplicateMap.find(AreaID);

                    if (Find != mAreaDuplicateMap.end())
                        pWorld->SetAreaAllowsPakDuplicates(iArea, Find->second);
                }

                // Set world name
                TString WorldName = MakeWorldName(pWorld->ID());
                pWorld->SetName(WorldName);
            }

            // Save raw resource + generate dependencies
            if (It->TypeInfo()->CanBeSerialized())
                It->Save(true);
            else
                It->UpdateDependencies();

            // Set flags, save metadata
            It->SaveMetadata(true);
        }
    }

    if (!mpProgress->ShouldCancel())
    {
        // All resources should have dependencies generated, so save the project files
        SCOPED_TIMER(SaveResourceDatabase);
#if EXPORT_COOKED
        bool ResDBSaveSuccess = mpStore->SaveDatabaseCache();
        ASSERT(ResDBSaveSuccess);
#endif
        bool ProjectSaveSuccess = mpProject->Save();
        ASSERT(ProjectSaveSuccess);
    }
}

void CGameExporter::ExportResource(SResourceInstance& rRes)
{
    if (!rRes.Exported)
    {
        std::vector<uint8> ResourceData;
        LoadResource(rRes, ResourceData);

        // Register resource and write to file
        TString Directory, Name;
        bool AutoDir, AutoName;

#if USE_ASSET_NAME_MAP
        mpNameMap->GetNameInfo(rRes.ResourceID, Directory, Name, AutoDir, AutoName);
#else
        Directory = mpStore->DefaultAssetDirectoryPath(mpStore->Game());
        Name = rRes.ResourceID.ToString();
#endif

        CResourceEntry *pEntry = mpStore->CreateNewResource(rRes.ResourceID,
                                                            CResTypeInfo::TypeForCookedExtension(mGame, rRes.ResourceType)->Type(),
                                                            Directory, Name, true);

        // Set flags
        pEntry->SetFlag(EResEntryFlag::IsBaseGameResource);
        pEntry->SetFlagEnabled(EResEntryFlag::AutoResDir, AutoDir);
        pEntry->SetFlagEnabled(EResEntryFlag::AutoResName, AutoName);

#if EXPORT_COOKED
        // Save cooked asset
        TString OutCookedPath = pEntry->CookedAssetPath();
        FileUtil::MakeDirectory(OutCookedPath.GetFileDirectory());
        CFileOutStream Out(OutCookedPath, EEndian::BigEndian);

        if (Out.IsValid())
            Out.WriteBytes(ResourceData.data(), ResourceData.size());

        ASSERT(pEntry->HasCookedVersion());
#endif

        rRes.Exported = true;
    }
}

TString CGameExporter::MakeWorldName(CAssetID WorldID)
{
    CResourceEntry *pWorldEntry = mpStore->FindEntry(WorldID);
    ASSERT(pWorldEntry && pWorldEntry->ResourceType() == EResourceType::World);

    // Find the original world name in the package resource names
    TString WorldName;

    for (uint32 iPkg = 0; iPkg < mpProject->NumPackages(); iPkg++)
    {
        CPackage *pPkg = mpProject->PackageByIndex(iPkg);

        for (uint32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);

            if (rkRes.ID == WorldID)
            {
                WorldName = rkRes.Name;

                if (WorldName.EndsWith("_NODEPEND"))
                    WorldName = WorldName.ChopBack(9);

                break;
            }
        }

        if (!WorldName.IsEmpty()) break;
    }

    // Fix up the name; remove date/time, leading exclamation points, etc
    if (!WorldName.IsEmpty())
    {
        // World names are basically formatted differently in every game...
        // MP1 demo - Remove ! from the beginning
        if (mGame == EGame::PrimeDemo)
        {
            if (WorldName.StartsWith('!'))
                WorldName = WorldName.ChopFront(1);
        }

        // MP1 - Remove prefix characters and ending date
        else if (mGame == EGame::Prime)
        {
            WorldName = WorldName.ChopFront(2);
            bool StartedDate = false;

            while (!WorldName.IsEmpty())
            {
                char Chr = WorldName.Back();

                if (!StartedDate && Chr >= '0' && Chr <= '9')
                    StartedDate = true;
                else if (StartedDate && Chr != '_' && (Chr < '0' || Chr > '9'))
                    break;

                WorldName = WorldName.ChopBack(1);
            }
        }

        // MP2 demo - Use text between the first and second underscores
        else if (mGame == EGame::EchoesDemo)
        {
            uint32 UnderscoreA = WorldName.IndexOf('_');
            uint32 UnderscoreB = WorldName.IndexOf('_', UnderscoreA + 1);

            if (UnderscoreA != UnderscoreB && UnderscoreA != -1 && UnderscoreB != -1)
                WorldName = WorldName.SubString(UnderscoreA + 1, UnderscoreB - UnderscoreA - 1);
        }

        // MP2 - Remove text before first underscore and after last underscore, strip remaining underscores (except multiplayer maps, which have one underscore)
        else if (mGame == EGame::Echoes)
        {
            uint32 FirstUnderscore = WorldName.IndexOf('_');
            uint32 LastUnderscore = WorldName.LastIndexOf('_');

            if (FirstUnderscore != LastUnderscore && FirstUnderscore != -1 && LastUnderscore != -1)
            {
                WorldName = WorldName.ChopBack(WorldName.Size() - LastUnderscore);
                WorldName = WorldName.ChopFront(FirstUnderscore + 1);
                WorldName.Remove('_');
            }
        }

        // MP3 proto - Remove ! from the beginning and all text after last underscore
        else if (mGame == EGame::CorruptionProto)
        {
            if (WorldName.StartsWith('!'))
                WorldName = WorldName.ChopFront(1);

            uint32 LastUnderscore = WorldName.LastIndexOf('_');
            WorldName = WorldName.ChopBack(WorldName.Size() - LastUnderscore);
        }

        // MP3 - Remove text after last underscore
        else if (mGame == EGame::Corruption)
        {
            uint32 LastUnderscore = WorldName.LastIndexOf('_');

            if (LastUnderscore != -1 && !WorldName.StartsWith("front_end_"))
                WorldName = WorldName.ChopBack(WorldName.Size() - LastUnderscore);
        }

        // DKCR - Remove text prior to first underscore
        else if (mGame == EGame::DKCReturns)
        {
            uint32 Underscore = WorldName.IndexOf('_');
            WorldName = WorldName.ChopFront(Underscore + 1);
        }
    }

    return WorldName;
}
