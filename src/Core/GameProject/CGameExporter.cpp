#include "CGameExporter.h"
#include "Core/GameProject/CResourceIterator.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CompressionUtil.h>
#include <Common/CScopedTimer.h>
#include <Common/FileUtil.h>
#include <Common/Serialization/CXMLWriter.h>
#include <tinyxml2.h>

#define COPY_DISC_DATA 1
#define LOAD_PAKS 1
#define SAVE_PACKAGE_DEFINITIONS 1
#define USE_ASSET_NAME_MAP 0
#define EXPORT_COOKED 1

CGameExporter::CGameExporter(const TString& rkInputDir, const TString& rkOutputDir)
{
    mGame = eUnknownGame;
    mGameDir = FileUtil::MakeAbsolute(rkInputDir);
    mExportDir = FileUtil::MakeAbsolute(rkOutputDir);

    mpProject = new CGameProject(mExportDir);
    mDiscDir = L"Disc\\";
    mWorldsDirName = L"Worlds\\";
}

#if PUBLIC_RELEASE
#error Fix export directory being cleared!
#endif

bool CGameExporter::Export()
{
    SCOPED_TIMER(ExportGame);

    FileUtil::CreateDirectory(mExportDir);
    FileUtil::ClearDirectory(mExportDir);

    // Initial analyze/copy of disc data
    CopyDiscData();

    // Create project
    mpProject = new CGameProject(this, mExportDir, mGame);
    mpProject->SetProjectName(CMasterTemplate::FindGameName(mGame));
    mpProject->SetActive();
    mpStore = mpProject->ResourceStore();
    mContentDir = mpStore->RawDir(false);
    mCookedDir = mpStore->CookedDir(false);

#if USE_ASSET_NAME_MAP
    mNameMap = CAssetNameMap::LoadAssetNames(mGame);
#endif

    // Export game data
    CResourceStore *pOldStore = gpResourceStore;
    gpResourceStore = mpStore;

    LoadPaks();
    ExportCookedResources();
    mpProject->AudioManager()->LoadAssets();
    ExportResourceEditorData();

    // Export finished!
    delete mpProject;
    gpResourceStore = pOldStore;

    return true;
}

void CGameExporter::LoadResource(const CAssetID& rkID, std::vector<u8>& rBuffer)
{
    SResourceInstance *pInst = FindResourceInstance(rkID);
    if (pInst) LoadResource(*pInst, rBuffer);
}

// ************ PROTECTED ************
void CGameExporter::CopyDiscData()
{
#if COPY_DISC_DATA
    SCOPED_TIMER(CopyDiscData);

    // Create Disc output folder
    FileUtil::CreateDirectory(mExportDir + mDiscDir);
#endif

    // Copy data
    TWideStringList DiscFiles;
    FileUtil::GetDirectoryContents(mGameDir, DiscFiles);

    for (auto It = DiscFiles.begin(); It != DiscFiles.end(); It++)
    {
        TWideString FullPath = *It;
        TWideString RelPath = FullPath.ChopFront(mGameDir.Size());

        // Exclude PakTool files and folders
        if (FullPath.GetFileName(false) == L"PakTool" || FullPath.GetFileName(false) == L"zlib1" || RelPath.Contains(L"-pak"))
            continue;

        // Hack to determine game
        if (mGame == eUnknownGame)
        {
            TWideString Name = FullPath.GetFileName(false);
            if      (Name == L"MetroidCWP")  mGame = ePrimeDemo;
            else if (Name == L"NESemu")      mGame = ePrime;
            else if (Name == L"PirateGun")   mGame = eEchoesDemo;
            else if (Name == L"AtomicBeta")  mGame = eEchoes;
            else if (Name == L"InGameAudio") mGame = eCorruptionProto;
            else if (Name == L"GuiDVD")      mGame = eCorruption;
            else if (Name == L"PreloadData") mGame = eReturns;
        }

        // Detect paks
        if (FullPath.GetFileExtension().ToLower() == L"pak")
            mPaks.push_back(FullPath);

#if COPY_DISC_DATA
        // Create directory
        TWideString OutFile = mExportDir + mDiscDir + RelPath;
        FileUtil::CreateDirectory(OutFile.GetFileDirectory());

        // Copy file
        if (FileUtil::IsFile(FullPath))
            FileUtil::CopyFile(FullPath, OutFile);
#endif
    }

    ASSERT(mGame != eUnknownGame);
}

// ************ RESOURCE LOADING ************
void CGameExporter::LoadPaks()
{
#if LOAD_PAKS
    SCOPED_TIMER(LoadPaks);

    for (auto It = mPaks.begin(); It != mPaks.end(); It++)
    {
        TWideString PakPath = *It;
        TWideString PakName = PakPath.GetFileName(false);
        TString CharPak = PakPath.ToUTF8();
        CFileInStream Pak(CharPak.ToStdString(), IOUtil::eBigEndian);

        if (!Pak.IsValid())
        {
            Log::Error("Couldn't open pak: " + CharPak);
            continue;
        }

        CPackage *pPackage = new CPackage(mpProject, CharPak.GetFileName(false), FileUtil::MakeRelative(PakPath.GetFileDirectory(), mGameDir));
        CResourceCollection *pCollection = pPackage->AddCollection("Default");

        // MP1-MP3Proto
        if (mGame < eCorruption)
        {
            u32 PakVersion = Pak.ReadLong();
            Pak.Seek(0x4, SEEK_CUR);
            ASSERT(PakVersion == 0x00030005);

            // Echoes demo disc has a pak that ends right here.
            if (!Pak.EoF())
            {
                u32 NumNamedResources = Pak.ReadLong();
                ASSERT(NumNamedResources > 0);

                for (u32 iName = 0; iName < NumNamedResources; iName++)
                {
                    CFourCC ResType = Pak.ReadLong();
                    CAssetID ResID(Pak, mGame);
                    u32 NameLen = Pak.ReadLong();
                    TString Name = Pak.ReadString(NameLen);
                    pCollection->AddResource(Name, ResID, ResType);
                }

                u32 NumResources = Pak.ReadLong();

                // Keep track of which areas have duplicate resources
                std::set<CAssetID> PakResourceSet;
                bool AreaHasDuplicates = true; // Default to true so that first area is always considered as having duplicates

                for (u32 iRes = 0; iRes < NumResources; iRes++)
                {
                    bool Compressed = (Pak.ReadLong() == 1);
                    CFourCC ResType = Pak.ReadLong();
                    CAssetID ResID(Pak, mGame);
                    u32 ResSize = Pak.ReadLong();
                    u32 ResOffset = Pak.ReadLong();

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
            u32 PakVersion = Pak.ReadLong();
            u32 PakHeaderLen = Pak.ReadLong();
            Pak.Seek(PakHeaderLen - 0x8, SEEK_CUR);
            ASSERT(PakVersion == 2);

            struct SPakSection {
                CFourCC Type; u32 Size;
            };
            std::vector<SPakSection> PakSections;

            u32 NumPakSections = Pak.ReadLong();
            ASSERT(NumPakSections == 3);

            for (u32 iSec = 0; iSec < NumPakSections; iSec++)
            {
                CFourCC Type = Pak.ReadLong();
                u32 Size = Pak.ReadLong();
                PakSections.push_back(SPakSection { Type, Size });
            }
            Pak.SeekToBoundary(64);

            for (u32 iSec = 0; iSec < NumPakSections; iSec++)
            {
                u32 Next = Pak.Tell() + PakSections[iSec].Size;

                // Named Resources
                if (PakSections[iSec].Type == "STRG")
                {
                    u32 NumNamedResources = Pak.ReadLong();

                    for (u32 iName = 0; iName < NumNamedResources; iName++)
                    {
                        TString Name = Pak.ReadString();
                        CFourCC ResType = Pak.ReadLong();
                        CAssetID ResID(Pak, mGame);
                        pCollection->AddResource(Name, ResID, ResType);
                    }
                }

                else if (PakSections[iSec].Type == "RSHD")
                {
                    ASSERT(PakSections[iSec + 1].Type == "DATA");
                    u32 DataStart = Next;
                    u32 NumResources = Pak.ReadLong();

                    // Keep track of which areas have duplicate resources
                    std::set<CAssetID> PakResourceSet;
                    bool AreaHasDuplicates = true; // Default to true so that first area is always considered as having duplicates

                    for (u32 iRes = 0; iRes < NumResources; iRes++)
                    {
                        bool Compressed = (Pak.ReadLong() == 1);
                        CFourCC Type = Pak.ReadLong();
                        CAssetID ResID(Pak, mGame);
                        u32 Size = Pak.ReadLong();
                        u32 Offset = DataStart + Pak.ReadLong();

                        if (mResourceMap.find(ResID) == mResourceMap.end())
                            mResourceMap[ResID] = SResourceInstance { PakPath, ResID, Type, Offset, Size, Compressed, false };

                        // Check for duplicate resources (unnecessary for DKCR)
                        if (mGame != eReturns)
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
        pPackage->Save();
#endif
    }
#endif
}

void CGameExporter::LoadResource(const SResourceInstance& rkResource, std::vector<u8>& rBuffer)
{
    CFileInStream Pak(rkResource.PakFile.ToUTF8().ToStdString(), IOUtil::eBigEndian);

    if (Pak.IsValid())
    {
        Pak.Seek(rkResource.PakOffset, SEEK_SET);

        // Handle compression
        if (rkResource.Compressed)
        {
            bool ZlibCompressed = (mGame <= eEchoesDemo || mGame == eReturns);

            if (mGame <= eCorruptionProto)
            {
                std::vector<u8> CompressedData(rkResource.PakSize);

                u32 UncompressedSize = Pak.ReadLong();
                rBuffer.resize(UncompressedSize);
                Pak.ReadBytes(CompressedData.data(), CompressedData.size());

                if (ZlibCompressed)
                {
                    u32 TotalOut;
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

                u32 NumBlocks = Pak.ReadLong();

                struct SCompressedBlock {
                    u32 CompressedSize; u32 UncompressedSize;
                };
                std::vector<SCompressedBlock> CompressedBlocks;

                u32 TotalUncompressedSize = 0;
                for (u32 iBlock = 0; iBlock < NumBlocks; iBlock++)
                {
                    u32 CompressedSize = (Pak.ReadLong() & 0x00FFFFFF);
                    u32 UncompressedSize = Pak.ReadLong();

                    TotalUncompressedSize += UncompressedSize;
                    CompressedBlocks.push_back( SCompressedBlock { CompressedSize, UncompressedSize } );
                }

                rBuffer.resize(TotalUncompressedSize);
                u32 Offset = 0;

                for (u32 iBlock = 0; iBlock < NumBlocks; iBlock++)
                {
                    u32 CompressedSize = CompressedBlocks[iBlock].CompressedSize;
                    u32 UncompressedSize = CompressedBlocks[iBlock].UncompressedSize;

                    // Block is compressed
                    if (CompressedSize != UncompressedSize)
                    {
                        std::vector<u8> CompressedData(CompressedBlocks[iBlock].CompressedSize);
                        Pak.ReadBytes(CompressedData.data(), CompressedData.size());

                        if (ZlibCompressed)
                        {
                            u32 TotalOut;
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
    {
        SCOPED_TIMER(ExportCookedResources);
        FileUtil::CreateDirectory(mCookedDir);

        for (auto It = mResourceMap.begin(); It != mResourceMap.end(); It++)
        {
            SResourceInstance& rRes = It->second;
            ExportResource(rRes);
        }
    }
    {
        SCOPED_TIMER(SaveResourceDatabase);
#if EXPORT_COOKED
        mpStore->SaveResourceDatabase();
#endif
        mpProject->Save();
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

        // todo: we're wasting a ton of time loading the same resources over and over because most resources automatically
        // load all their dependencies and then we just clear it out from memory even though we'll need it again later. we
        // should really be doing this by dependency order instead of by ID order.
        for (CResourceIterator It(mpStore); It; ++It)
        {
            if (!It->IsTransient())
            {
                // Worlds need to know which areas can have duplicates. We only have this info at export time.
                if (It->ResourceType() == eWorld)
                {
                    CWorld *pWorld = (CWorld*) It->Load();

                    for (u32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                    {
                        CAssetID AreaID = pWorld->AreaResourceID(iArea);
                        auto Find = mAreaDuplicateMap.find(AreaID);

                        if (Find != mAreaDuplicateMap.end())
                            pWorld->SetAreaAllowsPakDuplicates(iArea, Find->second);
                    }
                }

                // Save raw resource + generate dependencies
                It->Save(true);
            }
        }
    }
    {
        // All resources should have dependencies generated, so save the cache file
        SCOPED_TIMER(SaveResourceCacheData);
        mpStore->SaveCacheFile();
    }
}

void CGameExporter::ExportResource(SResourceInstance& rRes)
{
    if (!rRes.Exported)
    {
        std::vector<u8> ResourceData;
        LoadResource(rRes, ResourceData);

        // Register resource and write to file
        TString Directory, Name;
        mNameMap.GetNameInfo(rRes.ResourceID, Directory, Name);
        CResourceEntry *pEntry = mpStore->RegisterResource(rRes.ResourceID, CResource::ResTypeForExtension(rRes.ResourceType), Directory, Name);

#if EXPORT_COOKED
        // Save cooked asset
        TWideString OutCookedPath = pEntry->CookedAssetPath();
        FileUtil::CreateDirectory(OutCookedPath.GetFileDirectory());
        CFileOutStream Out(OutCookedPath.ToUTF8().ToStdString(), IOUtil::eBigEndian);

        if (Out.IsValid())
            Out.WriteBytes(ResourceData.data(), ResourceData.size());

        ASSERT(pEntry->HasCookedVersion());
#endif

        rRes.Exported = true;
    }
}
