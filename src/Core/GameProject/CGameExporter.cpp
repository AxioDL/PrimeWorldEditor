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
#include <tinyxml2.h>

#define COPY_DISC_DATA 1
#define LOAD_PAKS 1
#define SAVE_PACKAGE_DEFINITIONS 1
#define EXPORT_WORLDS 1
#define EXPORT_COOKED 1
#define EXPORT_CACHE 1

CGameExporter::CGameExporter(const TString& rkInputDir, const TString& rkOutputDir)
    : mStore(this)
{
    mGameDir = FileUtil::MakeAbsolute(rkInputDir);
    mExportDir = FileUtil::MakeAbsolute(rkOutputDir);

    mpProject = new CGameProject(mExportDir);
    mDiscDir = mpProject->DiscDir(true);
    mContentDir = mpProject->ContentDir(false);
    mCookedDir = mpProject->CookedDir(false);
    mWorldsDirName = L"Worlds\\";
    mStore.SetActiveProject(mpProject);
}

bool CGameExporter::Export()
{
    SCOPED_TIMER(ExportGame);

    CResourceStore *pOldStore = gpResourceStore;
    gpResourceStore = &mStore;
    FileUtil::CreateDirectory(mExportDir);
    FileUtil::ClearDirectory(mExportDir);

    CopyDiscData();
    LoadAssetList();
    LoadPaks();
    ExportWorlds();
    ExportCookedResources();

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
        if (FullPath.GetFileName(false) == L"PakTool" || FullPath.GetFileName() == L"zlib1" || RelPath.Contains(L"-pak"))
            continue;

        // Hack to determine game
        if (Game() == eUnknownVersion)
        {
            TWideString Name = FullPath.GetFileName(false);
            if      (Name == L"MetroidCWP")  SetGame(ePrimeDemo);
            else if (Name == L"NESemu")      SetGame(ePrime);
            else if (Name == L"PirateGun")   SetGame(eEchoesDemo);
            else if (Name == L"AtomicBeta")  SetGame(eEchoes);
            else if (Name == L"InGameAudio") SetGame(eCorruptionProto);
            else if (Name == L"GuiDVD")      SetGame(eCorruption);
            else if (Name == L"PreloadData") SetGame(eReturns);
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

    ASSERT(Game() != eUnknownVersion);
    mpProject->SetGame(Game());
    mpProject->SetProjectName(CMasterTemplate::FindGameName(Game()));
}

void CGameExporter::LoadAssetList()
{
    SCOPED_TIMER(LoadAssetList);

    // Determine the asset list to use
    TString ListFile = "../resources/list/AssetList";

    switch (Game())
    {
    case ePrimeDemo:        ListFile += "MP1Demo"; break;
    case ePrime:            ListFile += "MP1"; break;
    case eEchoesDemo:       ListFile += "MP2Demo"; break;
    case eEchoes:           ListFile += "MP2"; break;
    case eCorruptionProto:  ListFile += "MP3Proto"; break;
    case eCorruption:       ListFile += "MP3"; break;
    case eReturns:          ListFile += "DKCR"; break;
    default: ASSERT(false);
    }

    ListFile += ".xml";

    // Load list
    tinyxml2::XMLDocument List;
    List.LoadFile(*ListFile);

    if (List.Error())
    {
        Log::Error("Couldn't open asset list: " + ListFile);
        return;
    }

    tinyxml2::XMLElement *pRoot = List.FirstChildElement("AssetList");
    tinyxml2::XMLElement *pAsset = pRoot->FirstChildElement("Asset");

    while (pAsset)
    {
        u64 ResourceID = TString(pAsset->Attribute("ID")).ToInt64(16);

        tinyxml2::XMLElement *pDir = pAsset->FirstChildElement("Dir");
        TString Dir = pDir ? pDir->GetText() : "";

        tinyxml2::XMLElement *pName = pAsset->FirstChildElement("Name");
        TString Name = pName ? pName->GetText() : "";

        if (!Dir.EndsWith("/") && !Dir.EndsWith("\\")) Dir.Append("\\");
        SetResourcePath(ResourceID, Dir.ToUTF16(), Name.ToUTF16());

        pAsset = pAsset->NextSiblingElement("Asset");
    }
}

// ************ RESOURCE LOADING ************
void CGameExporter::LoadPaks()
{
#if LOAD_PAKS
    SCOPED_TIMER(LoadPaks);
    EIDLength IDLength = (Game() < eCorruptionProto ? e32Bit : e64Bit);

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
        if (Game() < eCorruption)
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
                    CAssetID ResID(Pak, IDLength);
                    u32 NameLen = Pak.ReadLong();
                    TString Name = Pak.ReadString(NameLen);
                    pCollection->AddResource(Name, ResID, ResType);
                    SetResourcePath(ResID.ToLongLong(), PakName + L"\\", Name.ToUTF16());
                }

                u32 NumResources = Pak.ReadLong();

                for (u32 iRes = 0; iRes < NumResources; iRes++)
                {
                    bool Compressed = (Pak.ReadLong() == 1);
                    CFourCC ResType = Pak.ReadLong();
                    CAssetID ResID(Pak, IDLength);
                    u32 ResSize = Pak.ReadLong();
                    u32 ResOffset = Pak.ReadLong();

                    u64 IntegralID = ResID.ToLongLong();
                    if (mResourceMap.find(IntegralID) == mResourceMap.end())
                        mResourceMap[IntegralID] = SResourceInstance { PakPath, ResID, ResType, ResOffset, ResSize, Compressed, false };
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
                        CAssetID ResID(Pak, IDLength);
                        pCollection->AddResource(Name, ResID, ResType);
                        SetResourcePath(ResID.ToLongLong(), PakName + L"\\", Name.ToUTF16());
                    }
                }

                else if (PakSections[iSec].Type == "RSHD")
                {
                    ASSERT(PakSections[iSec + 1].Type == "DATA");
                    u32 DataStart = Next;
                    u32 NumResources = Pak.ReadLong();

                    for (u32 iRes = 0; iRes < NumResources; iRes++)
                    {
                        bool Compressed = (Pak.ReadLong() == 1);
                        CFourCC Type = Pak.ReadLong();
                        CAssetID ResID(Pak, IDLength);
                        u32 Size = Pak.ReadLong();
                        u32 Offset = DataStart + Pak.ReadLong();

                        u64 IntegralID = ResID.ToLongLong();
                        if (mResourceMap.find(IntegralID) == mResourceMap.end())
                            mResourceMap[IntegralID] = SResourceInstance { PakPath, ResID, Type, Offset, Size, Compressed, false };
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
            bool ZlibCompressed = (Game() <= eEchoesDemo || Game() == eReturns);

            if (Game() <= eCorruptionProto)
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

void CGameExporter::ExportWorlds()
{
#if EXPORT_WORLDS
    SCOPED_TIMER(ExportWorlds);

    for (u32 iPak = 0; iPak < mpProject->NumPackages(); iPak++)
    {
        CPackage *pPak = mpProject->PackageByIndex(iPak);

        // Get output path. DKCR paks are stored in a Worlds folder so we should get the path relative to that so we don't have Worlds\Worlds\.
        // Other games have all paks in the game root dir so we're fine just taking the original root dir-relative directory.
        TWideString PakPath = pPak->Path();
        TWideString GameWorldsDir = PakPath.GetParentDirectoryPath(L"Worlds", false);

        if (!GameWorldsDir.IsEmpty())
            PakPath = FileUtil::MakeRelative(PakPath, GameWorldsDir);

        // Note since there's no collections in the cooked data we're guaranteed that every pak will have exactly one collection.
        CResourceCollection *pCollection = pPak->CollectionByIndex(0);

        for (u32 iRes = 0; iRes < pCollection->NumResources(); iRes++)
        {
            const SNamedResource& rkRes = pCollection->ResourceByIndex(iRes);

            if (rkRes.Type == "MLVL" && !rkRes.Name.EndsWith("NODEPEND"))
            {
                // Load world
                CWorld *pWorld = (CWorld*) mStore.LoadResource(rkRes.ID, rkRes.Type);

                if (!pWorld)
                {
                    Log::Error("Couldn't load world " + rkRes.Name + " from package " + pPak->Name() + "; unable to export");
                    continue;
                }

                // Export world
                TWideString Name = rkRes.Name.ToUTF16();
                TWideString WorldDir = mWorldsDirName + PakPath + FileUtil::SanitizeName(Name, true) + L"\\";
                FileUtil::CreateDirectory(mCookedDir + WorldDir);

                SResourceInstance *pInst = FindResourceInstance(rkRes.ID);
                ASSERT(pInst != nullptr);

                SetResourcePath(rkRes.ID, WorldDir, Name);
                ExportResource(*pInst);

                // Export areas
                for (u32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                {
                    // Determine area names
                    TWideString InternalAreaName = pWorld->AreaInternalName(iArea).ToUTF16();
                    bool HasInternalName = !InternalAreaName.IsEmpty();
                    if (!HasInternalName) InternalAreaName = TWideString::FromInt32(iArea, 2, 10);

                    TWideString GameAreaName;
                    CStringTable *pTable = pWorld->AreaName(iArea);
                    if (pTable) GameAreaName = pTable->String("ENGL", 0);
                    if (GameAreaName.IsEmpty()) GameAreaName = InternalAreaName;

                    // Export area
                    TWideString AreaDir = WorldDir + TWideString::FromInt32(iArea, 2, 10) + L"_" + FileUtil::SanitizeName(GameAreaName, true) + L"\\";
                    FileUtil::CreateDirectory(mCookedDir + AreaDir);

                    CAssetID AreaID = pWorld->AreaResourceID(iArea);
                    SResourceInstance *pInst = FindResourceInstance(AreaID);
                    ASSERT(pInst != nullptr);

                    SetResourcePath(AreaID, AreaDir, GameAreaName);
                    ExportResource(*pInst);
                }

                mStore.DestroyUnreferencedResources();
            }
        }
    }
#endif
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
        mStore.SaveResourceDatabase(mpProject->ResourceDBPath(false).ToUTF8());
#endif
        mpProject->Save();
    }
#if EXPORT_CACHE
    {
        SCOPED_TIMER(SaveCacheData);

        for (CResourceIterator It(&mStore); It; ++It)
        {
            if (!It->IsTransient())
            {
                It->UpdateDependencies();
                It->SaveCacheData();
            }
        }
    }
#endif
}

void CGameExporter::ExportResource(SResourceInstance& rRes)
{
    if (!rRes.Exported)
    {
        std::vector<u8> ResourceData;
        LoadResource(rRes, ResourceData);

        // Determine output path
        SResourcePath *pPath = FindResourcePath(rRes.ResourceID);
        TWideString OutName, OutDir;

        if (pPath)
        {
            OutName = pPath->Name;
            OutDir = pPath->Dir;
        }

        if (OutName.IsEmpty())  OutName = rRes.ResourceID.ToString().ToUTF16();
        if (OutDir.IsEmpty())   OutDir = L"Uncategorized\\";

        // Register resource and write to file
        CResourceEntry *pEntry = mStore.RegisterResource(rRes.ResourceID, CResource::ResTypeForExtension(rRes.ResourceType), OutDir, OutName);

#if EXPORT_COOKED
        // Cooked (todo: save raw)
        TWideString OutPath = pEntry->CookedAssetPath();
        FileUtil::CreateDirectory(OutPath.GetFileDirectory());
        CFileOutStream Out(OutPath.ToUTF8().ToStdString(), IOUtil::eBigEndian);

        if (Out.IsValid())
            Out.WriteBytes(ResourceData.data(), ResourceData.size());

        rRes.Exported = true;
        ASSERT(pEntry->HasCookedVersion());
#else
        (void) pEntry; // Prevent "unused local variable" compiler warning
#endif
    }
}
