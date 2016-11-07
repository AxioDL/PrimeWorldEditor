#include "CPackage.h"
#include "DependencyListBuilders.h"
#include "CGameProject.h"
#include "Core/Resource/Cooker/CWorldCooker.h"
#include <FileIO/FileIO.h>
#include <Common/AssertMacro.h>
#include <Common/CompressionUtil.h>
#include <Common/FileUtil.h>
#include <Common/Serialization/XML.h>

using namespace tinyxml2;

void CPackage::Load()
{
    TWideString DefPath = DefinitionPath(false);
    CXMLReader Reader(DefPath.ToUTF8());
    Serialize(Reader);
}

void CPackage::Save()
{
    TWideString DefPath = DefinitionPath(false);
    FileUtil::CreateDirectory(DefPath.GetFileDirectory());

    CXMLWriter Writer(DefPath.ToUTF8(), "PackageDefinition", 0, mpProject ? mpProject->Game() : eUnknownGame);
    Serialize(Writer);
}

void CPackage::Serialize(IArchive& rArc)
{
    rArc << SERIAL_CONTAINER("Collections", mCollections, "ResourceCollection");
}

void CPackage::Cook()
{
    // Build asset list
    CPackageDependencyListBuilder Builder(this);
    std::list<CAssetID> AssetList;
    Builder.BuildDependencyList(true, AssetList);
    Log::Write(TString::FromInt32(AssetList.size(), 0, 10) + " assets in pak");

    // Get named resources
    std::list<const SNamedResource*> NamedResources;

    for (u32 iCol = 0; iCol < mCollections.size(); iCol++)
    {
        CResourceCollection *pCol = mCollections[iCol];

        for (u32 iRes = 0; iRes < pCol->NumResources(); iRes++)
            NamedResources.push_back(&pCol->ResourceByIndex(iRes));
    }

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
    Pak.WriteLong(NamedResources.size());

    for (auto Iter = NamedResources.begin(); Iter != NamedResources.end(); Iter++)
    {
        const SNamedResource *pkRes = *Iter;
        pkRes->Type.Write(Pak);
        pkRes->ID.Write(Pak);
        Pak.WriteLong(pkRes->Name.Size());
        Pak.WriteString(pkRes->Name.ToStdString(), pkRes->Name.Size()); // Note: Explicitly specifying size means we don't write the terminating 0

        // TEMP: recook world
        if (pkRes->Type == "MLVL")
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(pkRes->ID);
            ASSERT(pEntry);
            CWorld *pWorld = (CWorld*) pEntry->Load();
            ASSERT(pWorld);
            CFileOutStream MLVL(pEntry->CookedAssetPath().ToStdString(), IOUtil::eBigEndian);
            ASSERT(MLVL.IsValid());
            CWorldCooker::CookMLVL(pWorld, MLVL);
        }
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
        CAssetID ID = *Iter;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
        ASSERT(pEntry != nullptr);

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

    Log::Write("Finished writing " + PakPath.ToUTF8());
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
            TString Extension = (pEntry ? "." + GetResourceCookedExtension(pEntry->ResourceType(), pEntry->Game()) : "");
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
            TString Extension = (pEntry ? "." + GetResourceCookedExtension(pEntry->ResourceType(), pEntry->Game()) : "");
            Log::Error("Extra resource: " + ID.ToString() + Extension);
        }
    }
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

CResourceCollection* CPackage::AddCollection(const TString& rkName)
{
    CResourceCollection *pCollection = new CResourceCollection(rkName);
    mCollections.push_back(pCollection);
    return pCollection;
}

void CPackage::RemoveCollection(CResourceCollection *pCollection)
{
    for (u32 iCol = 0; iCol < mCollections.size(); iCol++)
    {
        if (mCollections[iCol] == pCollection)
        {
            RemoveCollection(iCol);
            break;
        }
    }
}

void CPackage::RemoveCollection(u32 Index)
{
    ASSERT(Index < mCollections.size());
    auto Iter = mCollections.begin() + Index;
    delete *Iter;
    mCollections.erase(Iter);
}
