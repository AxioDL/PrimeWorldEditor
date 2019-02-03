#include "CPackage.h"
#include "DependencyListBuilders.h"
#include "CGameProject.h"
#include "Core/CompressionUtil.h"
#include "Core/Resource/Cooker/CWorldCooker.h"
#include <Common/Macros.h>
#include <Common/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/Serialization/XML.h>

using namespace tinyxml2;

bool CPackage::Load()
{
    TString DefPath = DefinitionPath(false);
    CXMLReader Reader(DefPath);

    if (Reader.IsValid())
    {
        Serialize(Reader);
        mCacheDirty = true;
        return true;
    }
    else return false;
}

bool CPackage::Save()
{
    TString DefPath = DefinitionPath(false);
    FileUtil::MakeDirectory(DefPath.GetFileDirectory());

    CXMLWriter Writer(DefPath, "PackageDefinition", 0, mpProject ? mpProject->Game() : EGame::Invalid);
    Serialize(Writer);
    return Writer.Save();
}

void CPackage::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("NeedsRecook", mNeedsRecook)
         << SerialParameter("NamedResources", mResources);
}

void CPackage::AddResource(const TString& rkName, const CAssetID& rkID, const CFourCC& rkType)
{
    mResources.push_back( SNamedResource { rkName, rkID, rkType } );
    mCacheDirty = true;
}

void CPackage::UpdateDependencyCache() const
{
    CPackageDependencyListBuilder Builder(this);
    std::list<CAssetID> AssetList;
    Builder.BuildDependencyList(false, AssetList);

    mCachedDependencies.clear();
    for (auto Iter = AssetList.begin(); Iter != AssetList.end(); Iter++)
        mCachedDependencies.insert(*Iter);

    mCacheDirty = false;
}

void CPackage::MarkDirty()
{
    if (!mNeedsRecook)
    {
        mNeedsRecook = true;
        Save();
        UpdateDependencyCache();
    }
}

void CPackage::Cook(IProgressNotifier *pProgress)
{
    SCOPED_TIMER(CookPackage);

    // Build asset list
    pProgress->Report(-1, -1, "Building dependency list");

    CPackageDependencyListBuilder Builder(this);
    std::list<CAssetID> AssetList;
    Builder.BuildDependencyList(true, AssetList);
    debugf("%d assets in %s.pak", AssetList.size(), *Name());

    // Write new pak
    TString PakPath = CookedPackagePath(false);
    CFileOutStream Pak(PakPath, EEndian::BigEndian);

    if (!Pak.IsValid())
    {
        errorf("Couldn't cook package %s; unable to open package for writing", *CookedPackagePath(true));
        return;
    }

    EGame Game = mpProject->Game();
    uint32 Alignment = (Game <= EGame::CorruptionProto ? 0x20 : 0x40);
    uint32 AlignmentMinusOne = Alignment - 1;

    uint32 TocOffset = 0;
    uint32 NamesSize = 0;
    uint32 ResTableOffset = 0;
    uint32 ResTableSize = 0;
    uint32 ResDataSize = 0;

    // Write MP1 pak header
    if (Game <= EGame::CorruptionProto)
    {
        Pak.WriteLong(0x00030005); // Major/Minor Version
        Pak.WriteLong(0); // Unknown

        // Named Resources
        Pak.WriteLong(mResources.size());

        for (auto Iter = mResources.begin(); Iter != mResources.end(); Iter++)
        {
            const SNamedResource& rkRes = *Iter;
            rkRes.Type.Write(Pak);
            rkRes.ID.Write(Pak);
            Pak.WriteSizedString(rkRes.Name);
        }
    }

    // Write MP3 pak header
    else
    {
        // Header
        Pak.WriteLong(2); // Version
        Pak.WriteLong(0x40); // Header size
        Pak.WriteToBoundary(0x40, 0); // We don't care about the MD5 hash; the game doesn't use it

        // PAK table of contents; write later
        TocOffset = Pak.Tell();
        Pak.WriteLong(0);
        Pak.WriteToBoundary(0x40, 0);

        // Named Resources
        uint32 NamesStart = Pak.Tell();
        Pak.WriteLong(mResources.size());

        for (auto Iter = mResources.begin(); Iter != mResources.end(); Iter++)
        {
            const SNamedResource& rkRes = *Iter;
            Pak.WriteString(rkRes.Name);
            rkRes.Type.Write(Pak);
            rkRes.ID.Write(Pak);
        }

        Pak.WriteToBoundary(0x40, 0);
        NamesSize = Pak.Tell() - NamesStart;
    }

    // Fill in resource table with junk, write later
    ResTableOffset = Pak.Tell();
    Pak.WriteLong(AssetList.size());
    CAssetID Dummy = CAssetID::InvalidID(Game);

    for (uint32 iRes = 0; iRes < AssetList.size(); iRes++)
    {
        Pak.WriteLongLong(0);
        Dummy.Write(Pak);
        Pak.WriteLongLong(0);
    }

    Pak.WriteToBoundary(Alignment, 0);
    ResTableSize = Pak.Tell() - ResTableOffset;

    // Start writing resources
    struct SResourceTableInfo
    {
        CResourceEntry *pEntry;
        uint32 Offset;
        uint32 Size;
        bool Compressed;
    };
    std::vector<SResourceTableInfo> ResourceTableData(AssetList.size());
    uint32 ResIdx = 0;
    uint32 ResDataOffset = Pak.Tell();

    for (auto Iter = AssetList.begin(); Iter != AssetList.end() && !pProgress->ShouldCancel(); Iter++, ResIdx++)
    {
        // Initialize entry, recook assets if needed
        uint32 AssetOffset = Pak.Tell();
        CAssetID ID = *Iter;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
        ASSERT(pEntry != nullptr);

        if (pEntry->NeedsRecook())
        {
            pProgress->Report(ResIdx, AssetList.size(), "Cooking asset: " + pEntry->Name() + "." + pEntry->CookedExtension());
            pEntry->Cook();
        }

        // Update progress bar
        if (ResIdx & 0x1 || ResIdx == AssetList.size() - 1)
        {
            pProgress->Report(ResIdx, AssetList.size(), TString::Format("Writing asset %d/%d: %s", ResIdx+1, AssetList.size(), *(pEntry->Name() + "." + pEntry->CookedExtension())));
        }

        // Update table info
        SResourceTableInfo& rTableInfo = ResourceTableData[ResIdx];
        rTableInfo.pEntry = pEntry;
        rTableInfo.Offset = (Game <= EGame::Echoes ? AssetOffset : AssetOffset - ResDataOffset);

        // Load resource data
        CFileInStream CookedAsset(pEntry->CookedAssetPath(), EEndian::BigEndian);
        ASSERT(CookedAsset.IsValid());
        uint32 ResourceSize = CookedAsset.Size();

        std::vector<uint8> ResourceData(ResourceSize);
        CookedAsset.ReadBytes(ResourceData.data(), ResourceData.size());

        // Check if this asset should be compressed; there are a few resource types that are
        // always compressed, and some types that are compressed if they're over a certain size
        EResourceType Type = pEntry->ResourceType();
        uint32 CompressThreshold = (Game <= EGame::CorruptionProto ? 0x400 : 0x80);

        bool ShouldAlwaysCompress = (Type == EResourceType::Texture || Type == EResourceType::Model ||
                                     Type == EResourceType::Skin || Type == EResourceType::AnimSet ||
                                     Type == EResourceType::Animation || Type == EResourceType::Font);

        if (Game >= EGame::Corruption)
        {
            ShouldAlwaysCompress = ShouldAlwaysCompress ||
                                   (Type == EResourceType::Character || Type == EResourceType::SourceAnimData ||
                                    Type == EResourceType::Scan || Type == EResourceType::AudioSample ||
                                    Type == EResourceType::StringTable || Type == EResourceType::AudioAmplitudeData ||
                                    Type == EResourceType::DynamicCollision);
        }

        bool ShouldCompressConditional = !ShouldAlwaysCompress &&
                (Type == EResourceType::Particle || Type == EResourceType::ParticleElectric ||
                 Type == EResourceType::ParticleSwoosh || Type == EResourceType::ParticleWeapon ||
                 Type == EResourceType::ParticleDecal || Type == EResourceType::ParticleCollisionResponse ||
                 Type == EResourceType::ParticleSpawn || Type == EResourceType::ParticleSorted ||
                 Type == EResourceType::BurstFireData);

        bool ShouldCompress = ShouldAlwaysCompress || (ShouldCompressConditional && ResourceSize >= CompressThreshold);

        // Write resource data to pak
        if (!ShouldCompress)
        {
            Pak.WriteBytes(ResourceData.data(), ResourceSize);
            rTableInfo.Compressed = false;
        }

        else
        {
            uint32 CompressedSize;
            std::vector<uint8> CompressedData(ResourceData.size() * 2);
            bool Success = false;

            if (Game <= EGame::EchoesDemo || Game == EGame::DKCReturns)
                Success = CompressionUtil::CompressZlib(ResourceData.data(), ResourceData.size(), CompressedData.data(), CompressedData.size(), CompressedSize);
            else
                Success = CompressionUtil::CompressLZOSegmented(ResourceData.data(), ResourceData.size(), CompressedData.data(), CompressedSize, false);

            // Make sure that the compressed data is actually smaller, accounting for padding + uncompressed size value
            if (Success)
            {
                uint32 CompressionHeaderSize = (Game <= EGame::CorruptionProto ? 4 : 0x10);
                uint32 PaddedUncompressedSize = (ResourceSize + AlignmentMinusOne) & ~AlignmentMinusOne;
                uint32 PaddedCompressedSize = (CompressedSize + CompressionHeaderSize + AlignmentMinusOne) & ~AlignmentMinusOne;
                Success = (PaddedCompressedSize < PaddedUncompressedSize);
            }

            // Write file to pak
            if (Success)
            {
                // Write MP1/2 compressed asset
                if (Game <= EGame::CorruptionProto)
                {
                    Pak.WriteLong(ResourceSize);
                }
                // Write MP3/DKCR compressed asset
                else
                {
                    // Note: Compressed asset data can be stored in multiple blocks. Normally, the only assets that make use of this are textures,
                    // which can store each separate component of the file (header, palette, image data) in separate blocks. However, some textures
                    // are stored in one block, and I've had no luck figuring out why. The game doesn't generally seem to care whether textures use
                    // multiple blocks or not, so for the sake of simplicity we compress everything to one block.
                    Pak.WriteFourCC( FOURCC('CMPD') );
                    Pak.WriteLong(1);
                    Pak.WriteLong(0xA0000000 | CompressedSize);
                    Pak.WriteLong(ResourceSize);
                }
                Pak.WriteBytes(CompressedData.data(), CompressedSize);
            }
            else
                Pak.WriteBytes(ResourceData.data(), ResourceSize);

            rTableInfo.Compressed = Success;
        }

        Pak.WriteToBoundary(Alignment, 0xFF);
        rTableInfo.Size = Pak.Tell() - AssetOffset;
    }
    ResDataSize = Pak.Tell() - ResDataOffset;

    // If we cancelled, don't finish writing the pak; delete the file instead and make sure the package is flagged for recook
    if (pProgress->ShouldCancel())
    {
        Pak.Close();
        FileUtil::DeleteFile(PakPath);
        mNeedsRecook = true;
    }

    else
    {
        // Write table of contents for real
        if (Game >= EGame::Corruption)
        {
            Pak.Seek(TocOffset, SEEK_SET);
            Pak.WriteLong(3); // Always 3 pak sections
            Pak.WriteFourCC( FOURCC('STRG') );
            Pak.WriteLong(NamesSize);
            Pak.WriteFourCC( FOURCC('RSHD') );
            Pak.WriteLong(ResTableSize);
            Pak.WriteFourCC( FOURCC('DATA') );
            Pak.WriteLong(ResDataSize);
        }

        // Write resource table for real
        Pak.Seek(ResTableOffset+4, SEEK_SET);

        for (uint32 iRes = 0; iRes < AssetList.size(); iRes++)
        {
            const SResourceTableInfo& rkInfo = ResourceTableData[iRes];
            CResourceEntry *pEntry = rkInfo.pEntry;

            Pak.WriteLong( rkInfo.Compressed ? 1 : 0 );
            pEntry->CookedExtension().Write(Pak);
            pEntry->ID().Write(Pak);
            Pak.WriteLong(rkInfo.Size);
            Pak.WriteLong(rkInfo.Offset);
        }

        // Clear recook flag
        mNeedsRecook = false;
        debugf("Finished writing %s", *PakPath);
    }

    Save();

    // Update resource store in case we recooked any assets
    mpProject->ResourceStore()->ConditionalSaveStore();
}

void CPackage::CompareOriginalAssetList(const std::list<CAssetID>& rkNewList)
{
    // Debug - take the newly generated rkNewList and compare it with the asset list
    // from the original pak, and print info about any extra or missing resources
    // Build a set out of the generated list
    std::set<CAssetID> NewListSet;

    for (auto Iter = rkNewList.begin(); Iter != rkNewList.end(); Iter++)
        NewListSet.insert(*Iter);

    // Read the original pak
    TString CookedPath = CookedPackagePath(false);
    CFileInStream Pak(CookedPath, EEndian::BigEndian);

    if (!Pak.IsValid() || Pak.Size() == 0)
    {
        errorf("Failed to compare to original asset list; couldn't open the original pak");
        return;
    }

    // Determine pak version
    uint32 PakVersion = Pak.ReadLong();
    std::set<CAssetID> OldListSet;

    // Read MP1/2 pak
    if (PakVersion == 0x00030005)
    {
        Pak.Seek(0x4, SEEK_CUR);
        uint32 NumNamedResources = Pak.ReadLong();

        for (uint32 iName = 0; iName < NumNamedResources; iName++)
        {
            Pak.Seek(0x8, SEEK_CUR);
            uint32 NameLen = Pak.ReadLong();
            Pak.Seek(NameLen, SEEK_CUR);
        }

        // Build a set out of the original pak resource list
        uint32 NumResources = Pak.ReadLong();

        for (uint32 iRes = 0; iRes < NumResources; iRes++)
        {
            Pak.Seek(0x8, SEEK_CUR);
            OldListSet.insert( CAssetID(Pak, k32Bit) );
            Pak.Seek(0x8, SEEK_CUR);
        }
    }

    // Read MP3/DKCR pak
    else
    {
        ASSERT(PakVersion == 0x2);

        // Skip named resources
        Pak.Seek(0x44, SEEK_SET);
        CFourCC StringSecType = Pak.ReadLong();
        uint32 StringSecSize = Pak.ReadLong();
        ASSERT(StringSecType == "STRG");

        Pak.Seek(0x80 + StringSecSize, SEEK_SET);

        // Read resource table
        uint32 NumResources = Pak.ReadLong();

        for (uint32 iRes = 0; iRes < NumResources; iRes++)
        {
            Pak.Seek(0x8, SEEK_CUR);
            OldListSet.insert( CAssetID(Pak, k64Bit) );
            Pak.Seek(0x8, SEEK_CUR);
        }
    }

    // Check for missing resources in the new list
    for (auto Iter = OldListSet.begin(); Iter != OldListSet.end(); Iter++)
    {
        CAssetID ID = *Iter;

        if (NewListSet.find(ID) == NewListSet.end())
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
            TString Extension = (pEntry ? "." + pEntry->CookedExtension() : "");
            warnf("Missing resource: %s%s", *ID.ToString(), *Extension);
        }
    }

    // Check for extra resources in the new list
    for (auto Iter = NewListSet.begin(); Iter != NewListSet.end(); Iter++)
    {
        CAssetID ID = *Iter;

        if (OldListSet.find(ID) == OldListSet.end())
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
            TString Extension = (pEntry ? "." + pEntry->CookedExtension() : "");
            warnf("Extra resource: %s%s", *ID.ToString(), *Extension);
        }
    }
}

bool CPackage::ContainsAsset(const CAssetID& rkID) const
{
    if (mCacheDirty)
        UpdateDependencyCache();

    return mCachedDependencies.find(rkID) != mCachedDependencies.end();
}

TString CPackage::DefinitionPath(bool Relative) const
{
    TString RelPath = mPakPath + mPakName + ".pkd";
    return Relative ? RelPath : mpProject->PackagesDir(false) + RelPath;
}

TString CPackage::CookedPackagePath(bool Relative) const
{
    TString RelPath = mPakPath + mPakName + ".pak";
    return Relative ? RelPath : mpProject->DiscFilesystemRoot(false) + RelPath;
}
