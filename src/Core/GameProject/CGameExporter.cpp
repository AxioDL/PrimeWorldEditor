#include "CGameExporter.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CompressionUtil.h>
#include <Common/FileUtil.h>

#define COPY_DISC_DATA 1
#define LOAD_PAKS 1
#define EXPORT_COOKED 1

CGameExporter::CGameExporter(const TString& rkInputDir, const TString& rkOutputDir)
    : mGameDir( FileUtil::MakeAbsolute(rkInputDir) )
    , mExportDir( FileUtil::MakeAbsolute(rkOutputDir) )
    , mDiscDir(mExportDir + L"Disc\\")
    , mCookedResDir(mExportDir + L"Cooked\\Resources\\")
    , mCookedWorldsDir(mExportDir + L"Cooked\\Worlds\\")
    , mRawResDir(mExportDir + L"Raw\\Resources\\")
    , mRawWorldsDir(mExportDir + L"Raw\\Worlds\\")
    , mpProject(new CGameProject)
{
}

bool CGameExporter::Export()
{
    FileUtil::CreateDirectory(mExportDir);
    CopyDiscData();
    LoadPaks();
    ExportCookedResources();
    return true;
}

// ************ PROTECTED ************
void CGameExporter::CopyDiscData()
{
#if COPY_DISC_DATA
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
        if (FullPath.GetFileExtension() == L"pak")
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

// ************ RESOURCE LOADING ************
void CGameExporter::LoadPaks()
{
#if LOAD_PAKS
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

            CPackage *pPackage = new CPackage(CharPak.GetFileName(false));

            // MP1-MP3Proto
            if (Game() < eCorruption)
            {
                u32 PakVersion = Pak.ReadLong();
                Pak.Seek(0x4, SEEK_CUR);
                ASSERT(PakVersion == 0x00030005);

                u32 NumNamedResources = Pak.ReadLong();
                ASSERT(NumNamedResources > 0);

                for (u32 iName = 0; iName < NumNamedResources; iName++)
                {
                    Pak.Seek(0x4, SEEK_CUR); // Skip resource type
                    CUniqueID ResID(Pak, IDLength);
                    u32 NameLen = Pak.ReadLong();
                    TString Name = Pak.ReadString(NameLen);
                    pPackage->AddNamedResource(Name, ResID);
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
                        mResourceMap[IntegralID] = SResourceInstance { PakPath, ResID, ResType, ResOffset, ResSize, Compressed };
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
                            Pak.Seek(0x4, SEEK_CUR); // Skip type
                            CUniqueID ResID(Pak, IDLength);
                            pPackage->AddNamedResource(Name, ResID);
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
                                mResourceMap[IntegralID] = SResourceInstance { PakPath, ResID, Type, Offset, Size, Compressed };
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

void CGameExporter::LoadPakResource(const SResourceInstance& rkResource, std::vector<u8>& rBuffer)
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

void CGameExporter::ExportCookedResources()
{
#if EXPORT_COOKED
    FileUtil::CreateDirectory(mCookedResDir);

    for (auto It = mResourceMap.begin(); It != mResourceMap.end(); It++)
    {
        const SResourceInstance& rkRes = It->second;
        std::vector<u8> ResourceData;
        LoadPakResource(rkRes, ResourceData);

        TString OutName = rkRes.ResourceID.ToString() + "." + rkRes.ResourceType.ToString();
        TString OutPath = mCookedResDir.ToUTF8() + "/" + OutName;
        CFileOutStream Out(OutPath.ToStdString(), IOUtil::eBigEndian);

        if (Out.IsValid())
            Out.WriteBytes(ResourceData.data(), ResourceData.size());
    }
#endif
}
