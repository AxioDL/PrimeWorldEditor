#include "CPackage.h"
#include "DependencyListBuilders.h"
#include "CGameProject.h"
#include "Core/CompressionUtil.h"
#include "Core/Resource/Cooker/CWorldCooker.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <Common/Serialization/XML.h>

using namespace tinyxml2;

bool CPackage::Load()
{
    TWideString DefPath = DefinitionPath(false);
    CXMLReader Reader(DefPath.ToUTF8());

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
    TWideString DefPath = DefinitionPath(false);
    FileUtil::MakeDirectory(DefPath.GetFileDirectory());

    CXMLWriter Writer(DefPath.ToUTF8(), "PackageDefinition", 0, mpProject ? mpProject->Game() : eUnknownGame);
    Serialize(Writer);
    return Writer.Save();
}

void CPackage::Serialize(IArchive& rArc)
{
    rArc << SERIAL("NeedsRecook", mNeedsRecook)
         << SERIAL_CONTAINER("NamedResources", mResources, "Resource");
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

void CPackage::Cook()
{
    // Build asset list
    CPackageDependencyListBuilder Builder(this);
    std::list<CAssetID> AssetList;
    Builder.BuildDependencyList(true, AssetList);
    Log::Write(TString::FromInt32(AssetList.size(), 0, 10) + " assets in " + Name() + ".pak");

    // Write new pak
    TWideString PakPath = CookedPackagePath(false);
    CFileOutStream Pak(PakPath.ToUTF8().ToStdString(), IOUtil::eBigEndian);

    if (!Pak.IsValid())
    {
        Log::Error("Couldn't cook package " + CookedPackagePath(true).ToUTF8() + "; unable to open package for writing");
        return;
    }

    // todo: MP3/DKCR pak format support
    Pak.WriteLong(0x00030005); // Major/Minor Version
    Pak.WriteLong(0); // Unknown

    // Named Resources
    Pak.WriteLong(mResources.size());

    for (auto Iter = mResources.begin(); Iter != mResources.end(); Iter++)
    {
        const SNamedResource& rkRes = *Iter;
        rkRes.Type.Write(Pak);
        rkRes.ID.Write(Pak);
        Pak.WriteLong(rkRes.Name.Size());
        Pak.WriteString(rkRes.Name.ToStdString(), rkRes.Name.Size()); // Note: Explicitly specifying size means we don't write the terminating 0
    }

    // Fill in table of contents with junk, write later
    Pak.WriteLong(AssetList.size());
    u32 TocOffset = Pak.Tell();

    for (u32 iRes = 0; iRes < AssetList.size(); iRes++)
    {
        Pak.WriteLongLong(0);
        Pak.WriteLongLong(0);
        Pak.WriteLong(0);
    }

    Pak.WriteToBoundary(32, 0);

    // Start writing resources
    struct SResourceTocInfo
    {
        CResourceEntry *pEntry;
        u32 Offset;
        u32 Size;
        bool Compressed;
    };
    std::vector<SResourceTocInfo> ResourceTocData(AssetList.size());
    u32 ResIdx = 0;

    for (auto Iter = AssetList.begin(); Iter != AssetList.end(); Iter++, ResIdx++)
    {
        // Initialize entry, recook assets if needed
        CAssetID ID = *Iter;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
        ASSERT(pEntry != nullptr);

        if (pEntry->NeedsRecook())
            pEntry->Cook();

        SResourceTocInfo& rTocInfo = ResourceTocData[ResIdx];
        rTocInfo.pEntry = pEntry;
        rTocInfo.Offset = Pak.Tell();

        // Load resource data
        CFileInStream CookedAsset(pEntry->CookedAssetPath().ToStdString(), IOUtil::eBigEndian);
        ASSERT(CookedAsset.IsValid());
        u32 ResourceSize = CookedAsset.Size();

        std::vector<u8> ResourceData(ResourceSize);
        CookedAsset.ReadBytes(ResourceData.data(), ResourceData.size());

        // Check if this asset should be compressed; there are a few resource types that are
        // always compressed, and some types that are compressed if they're over a certain size
        EResType Type = pEntry->ResourceType();

        bool ShouldAlwaysCompress = (Type == eTexture || Type == eModel || Type == eSkin ||
                                     Type == eAnimSet || Type == eAnimation || Type == eFont);

        bool ShouldCompressConditional = !ShouldAlwaysCompress &&
                (Type == eParticle || Type == eParticleElectric || Type == eParticleSwoosh ||
                 Type == eParticleWeapon || Type == eParticleDecal || Type == eParticleCollisionResponse);

        bool ShouldCompress = ShouldAlwaysCompress || (ShouldCompressConditional && ResourceSize >= 0x400);

        // Write resource data to pak
        if (!ShouldCompress)
        {
            Pak.WriteBytes(ResourceData.data(), ResourceSize);
            rTocInfo.Compressed = false;
        }

        else
        {
            u32 CompressedSize;
            std::vector<u8> CompressedData(ResourceData.size() * 2);
            bool Success = false;

            if (mpProject->Game() <= eEchoesDemo)
                Success = CompressionUtil::CompressZlib(ResourceData.data(), ResourceData.size(), CompressedData.data(), CompressedData.size(), CompressedSize);
            else
                Success = CompressionUtil::CompressLZOSegmented(ResourceData.data(), ResourceData.size(), CompressedData.data(), CompressedSize, false);

            // Make sure that the compressed data is actually smaller, accounting for padding + uncompressed size value
            if (Success)
            {
                u32 PaddedUncompressedSize = (ResourceSize + 0x1F) & ~0x1F;
                u32 PaddedCompressedSize = (CompressedSize + 4 + 0x1F) & ~0x1F;
                Success = (PaddedCompressedSize < PaddedUncompressedSize);
            }

            // Write file to pak
            if (Success)
            {
                Pak.WriteLong(ResourceSize);
                Pak.WriteBytes(CompressedData.data(), CompressedSize);
            }
            else
                Pak.WriteBytes(ResourceData.data(), ResourceSize);

            rTocInfo.Compressed = Success;
        }

        Pak.WriteToBoundary(32, 0xFF);
        rTocInfo.Size = Pak.Tell() - rTocInfo.Offset;
    }

    // Write table of contents for real
    Pak.Seek(TocOffset, SEEK_SET);

    for (u32 iRes = 0; iRes < AssetList.size(); iRes++)
    {
        const SResourceTocInfo& rkTocInfo = ResourceTocData[iRes];
        CResourceEntry *pEntry = rkTocInfo.pEntry;

        Pak.WriteLong( rkTocInfo.Compressed ? 1 : 0 );
        pEntry->CookedExtension().Write(Pak);
        pEntry->ID().Write(Pak);
        Pak.WriteLong(rkTocInfo.Size);
        Pak.WriteLong(rkTocInfo.Offset);
    }

    mNeedsRecook = false;
    Save();
    Log::Write("Finished writing " + PakPath.ToUTF8());

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
    TWideString CookedPath = CookedPackagePath(false);
    CFileInStream Pak(CookedPath.ToUTF8().ToStdString(), IOUtil::eBigEndian);

    if (!Pak.IsValid() || Pak.Size() == 0)
    {
        Log::Error("Failed to compare to original asset list; couldn't open the original pak");
        return;
    }

    // Skip past header + named resources
    u32 PakVersion = Pak.ReadLong();
    ASSERT(PakVersion == 0x00030005);
    Pak.Seek(0x4, SEEK_CUR);
    u32 NumNamedResources = Pak.ReadLong();

    for (u32 iName = 0; iName < NumNamedResources; iName++)
    {
        Pak.Seek(0x8, SEEK_CUR);
        u32 NameLen = Pak.ReadLong();
        Pak.Seek(NameLen, SEEK_CUR);
    }

    // Build a set out of the original pak resource list
    u32 NumResources = Pak.ReadLong();
    std::set<CAssetID> OldListSet;

    for (u32 iRes = 0; iRes < NumResources; iRes++)
    {
        Pak.Seek(0x8, SEEK_CUR);
        OldListSet.insert( CAssetID(Pak, e32Bit) );
        Pak.Seek(0x8, SEEK_CUR);
    }

    // Check for missing resources in the new list
    for (auto Iter = OldListSet.begin(); Iter != OldListSet.end(); Iter++)
    {
        CAssetID ID = *Iter;

        if (NewListSet.find(ID) == NewListSet.end())
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
            TString Extension = (pEntry ? "." + pEntry->CookedExtension() : "");
            Log::Error("Missing resource: " + ID.ToString() + Extension);
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
            Log::Error("Extra resource: " + ID.ToString() + Extension);
        }
    }
}

bool CPackage::ContainsAsset(const CAssetID& rkID) const
{
    if (mCacheDirty)
        UpdateDependencyCache();

    return mCachedDependencies.find(rkID) != mCachedDependencies.end();
}

TWideString CPackage::DefinitionPath(bool Relative) const
{
    TWideString RelPath = mPakPath + mPakName.ToUTF16() + L".pkd";
    return Relative ? RelPath : mpProject->PackagesDir(false) + RelPath;
}

TWideString CPackage::CookedPackagePath(bool Relative) const
{
    TWideString RelPath = mPakPath + mPakName.ToUTF16() + L".pak";
    return Relative ? RelPath : mpProject->DiscDir(false) + RelPath;
}
