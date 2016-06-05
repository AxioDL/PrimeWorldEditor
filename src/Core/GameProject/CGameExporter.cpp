#include "CGameExporter.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/CWorld.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CompressionUtil.h>
#include <Common/CScopedTimer.h>
#include <Common/FileUtil.h>
#include <tinyxml2.h>

#define COPY_DISC_DATA 0
#define LOAD_PAKS 1
#define EXPORT_WORLDS 1
#define EXPORT_COOKED 1

CGameExporter::CGameExporter(const TString& rkInputDir, const TString& rkOutputDir)
{
    mGameDir = FileUtil::MakeAbsolute(rkInputDir);
    mExportDir = FileUtil::MakeAbsolute(rkOutputDir);

    mpProject = new CGameProject(mExportDir);
    mDiscDir = mpProject->DiscDir(true);
    mResDir = mpProject->ResourcesDir(true);
    mWorldsDir = mpProject->WorldsDir(true);
    mCookedDir = mpProject->CookedDir(false);
    mCookedResDir = mpProject->CookedResourcesDir(true);
    mCookedWorldsDir = mpProject->CookedWorldsDir(true);
}

bool CGameExporter::Export()
{
    SCOPED_TIMER(ExportGame);
    gResCache.SetGameExporter(this);
    FileUtil::CreateDirectory(mExportDir);
    FileUtil::ClearDirectory(mExportDir);

    CopyDiscData();
    LoadAssetList();
    LoadPaks();
    ExportWorlds();
    ExportCookedResources();

    gResCache.SetGameExporter(nullptr);
    return true;
}

void CGameExporter::LoadResource(const CUniqueID& rkID, std::vector<u8>& rBuffer)
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
    FileUtil::CreateDirectory(mDiscDir);
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
        {
            if (FullPath.GetFileName(false).StartsWith(L"Metroid", false) || RelPath.Contains(L"Worlds", false))
                mWorldPaks.push_back(FullPath);
            else
                mResourcePaks.push_back(FullPath);
        }

#if COPY_DISC_DATA
        // Create directory
        TWideString OutFile = mDiscDir + RelPath;
        FileUtil::CreateDirectory(OutFile.GetFileDirectory());

        // Copy file
        if (FileUtil::IsFile(FullPath))
            FileUtil::CopyFile(FullPath, OutFile);
#endif
    }

    ASSERT(Game() != eUnknownVersion);
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
        SetResourcePath(ResourceID, mResDir + Dir.ToUTF16(), Name.ToUTF16());

        pAsset = pAsset->NextSiblingElement("Asset");
    }
}

// ************ RESOURCE LOADING ************
void CGameExporter::LoadPaks()
{
#if LOAD_PAKS
    SCOPED_TIMER(LoadPaks);

    for (u32 iList = 0; iList < 2; iList++)
    {
        const TWideStringList& rkList = (iList == 0 ? mWorldPaks : mResourcePaks);
        bool IsWorldPak = (iList == 0);
        EUIDLength IDLength = (Game() < eCorruptionProto ? e32Bit : e64Bit);

        for (auto It = rkList.begin(); It != rkList.end(); It++)
        {
            TWideString PakPath = *It;
            TString CharPak = PakPath.ToUTF8();
            CFileInStream Pak(CharPak.ToStdString(), IOUtil::eBigEndian);

            if (!Pak.IsValid())
            {
                Log::Error("Couldn't open pak: " + CharPak);
                continue;
            }

            CPackage *pPackage = new CPackage(CharPak.GetFileName(false), FileUtil::MakeRelative(PakPath.GetFileDirectory(), mExportDir));

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
                        CUniqueID ResID(Pak, IDLength);
                        u32 NameLen = Pak.ReadLong();
                        TString Name = Pak.ReadString(NameLen);
                        pPackage->AddNamedResource(Name, ResID, ResType);
                    }

                    u32 NumResources = Pak.ReadLong();

                    for (u32 iRes = 0; iRes < NumResources; iRes++)
                    {
                        bool Compressed = (Pak.ReadLong() == 1);
                        CFourCC ResType = Pak.ReadLong();
                        CUniqueID ResID(Pak, IDLength);
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
                            CUniqueID ResID(Pak, IDLength);
                            pPackage->AddNamedResource(Name, ResID, ResType);
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
                            CUniqueID ResID(Pak, IDLength);
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

            // Add package to project
            mpProject->AddPackage(pPackage, IsWorldPak);
        }
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
    //CResourceDatabase *pResDB = mpProject->ResourceDatabase();

    for (u32 iPak = 0; iPak < mpProject->NumWorldPaks(); iPak++)
    {
        CPackage *pPak = mpProject->WorldPakByIndex(iPak);

        // Get output path. DKCR paks are stored in a Worlds folder so we should get the path relative to that so we don't have Worlds\Worlds\.
        // Other games have all paks in the game root dir so we're fine just taking the original root dir-relative directory.
        TWideString PakPath = pPak->PakPath();
        TWideString WorldsDir = PakPath.GetParentDirectoryPath(L"Worlds", false);

        if (!WorldsDir.IsEmpty())
            PakPath = FileUtil::MakeRelative(PakPath, WorldsDir);

        for (u32 iRes = 0; iRes < pPak->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPak->NamedResourceByIndex(iRes);

            if (rkRes.Type == "MLVL" && !rkRes.Name.EndsWith("NODEPEND"))
            {
                TResPtr<CWorld> pWorld = (CWorld*) gResCache.GetResource(rkRes.ID, rkRes.Type);

                if (!pWorld)
                {
                    Log::Error("Couldn't load world " + rkRes.Name + " from package " + pPak->PakName() + "; unable to export");
                    continue;
                }

                // Export world
                TWideString Name = rkRes.Name.ToUTF16();
                TWideString WorldDir = mWorldsDir + PakPath + FileUtil::SanitizeName(Name, true) + L"\\";
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
                    if (InternalAreaName.IsEmpty()) InternalAreaName = TWideString::FromInt32(iArea, 2, 10);

                    TWideString GameAreaName;
                    CStringTable *pTable = pWorld->AreaName(iArea);
                    if (pTable) GameAreaName = pTable->String("ENGL", 0);
                    if (GameAreaName.IsEmpty()) GameAreaName = InternalAreaName;

                    // Load area
                    CUniqueID AreaID = pWorld->AreaResourceID(iArea);
                    CGameArea *pArea = (CGameArea*) gResCache.GetResource(AreaID, "MREA");

                    if (!pArea)
                    {
                        Log::Error("Unable to export area " + GameAreaName.ToUTF8() + " from world " + rkRes.Name + "; couldn't load area");
                        continue;
                    }

                    // Export area
                    TWideString AreaDir = WorldDir + TWideString::FromInt32(iArea, 2, 10) + L"_" + FileUtil::SanitizeName(GameAreaName, true) + L"\\";
                    FileUtil::CreateDirectory(mCookedDir + AreaDir);

                    SResourceInstance *pInst = FindResourceInstance(AreaID);
                    ASSERT(pInst != nullptr);

                    SetResourcePath(AreaID, AreaDir, InternalAreaName);
                    ExportResource(*pInst);
                }

                gResCache.Clean();
            }

            else
            {
                Log::Error("Unexpected named resource type in world pak: " + rkRes.Type.ToString());
            }
        }
    }
#endif
}

void CGameExporter::ExportCookedResources()
{
#if EXPORT_COOKED
    CResourceDatabase *pResDB = mpProject->ResourceDatabase();
    {
        SCOPED_TIMER(ExportCookedResources);
        FileUtil::CreateDirectory(mCookedDir + mResDir);

        for (auto It = mResourceMap.begin(); It != mResourceMap.end(); It++)
        {
            SResourceInstance& rRes = It->second;
            ExportResource(rRes);
        }
    }
    {
        SCOPED_TIMER(SaveResourceDatabase);
        pResDB->Save(this->mExportDir.ToUTF8() + "ResourceDatabase.rdb");
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
        TString OutName, OutDir;

        if (pPath)
        {
            OutName = pPath->Name.ToUTF8();
            OutDir = pPath->Dir.ToUTF8();
        }

        if (OutName.IsEmpty())  OutName = rRes.ResourceID.ToString();
        if (OutDir.IsEmpty())   OutDir = mResDir;

        // Write to file
        FileUtil::CreateDirectory(mCookedDir + OutDir.ToUTF16());
        TString OutPath = mCookedDir.ToUTF8() + OutDir + OutName + "." + rRes.ResourceType.ToString();
        CFileOutStream Out(OutPath.ToStdString(), IOUtil::eBigEndian);

        if (Out.IsValid())
            Out.WriteBytes(ResourceData.data(), ResourceData.size());

        // Add to resource DB
        mpProject->ResourceDatabase()->RegisterResource(rRes.ResourceID, OutDir, OutName, CResource::ResTypeForExtension(rRes.ResourceType));
        rRes.Exported = true;
    }
}
