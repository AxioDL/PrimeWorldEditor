#include "CResourceStore.h"
#include "CGameExporter.h"
#include "CGameProject.h"
#include "CResourceIterator.h"
#include "Core/Resource/CResource.h"
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/Serialization/Binary.h>
#include <Common/Serialization/XML.h>
#include <tinyxml2.h>

using namespace tinyxml2;
CResourceStore *gpResourceStore = new CResourceStore;

CResourceStore::CResourceStore()
    : mpProj(nullptr)
    , mpProjectRoot(nullptr)
    , mpExporter(nullptr)
{}

CResourceStore::CResourceStore(CGameExporter *pExporter)
    : mpProj(nullptr)
    , mpProjectRoot(nullptr)
    , mpExporter(pExporter)
{}

CResourceStore::~CResourceStore()
{
    CloseActiveProject();
    DestroyUnreferencedResources();

    for (auto It = mResourceEntries.begin(); It != mResourceEntries.end(); It++)
        delete It->second;

    for (auto It = mTransientRoots.begin(); It != mTransientRoots.end(); It++)
        delete *It;
}

void CResourceStore::SerializeResourceDatabase(IArchive& rArc)
{
    struct SDatabaseResource
    {
        CAssetID ID;
        CFourCC Type;
        TWideString Directory;
        TWideString Name;

        void Serialize(IArchive& rArc)
        {
            rArc << SERIAL_AUTO(ID) << SERIAL_AUTO(Type) << SERIAL_AUTO(Directory) << SERIAL_AUTO(Name);
        }
    };
    std::vector<SDatabaseResource> Resources;

    // Populate resource list
    if (!rArc.IsReader())
    {
        Resources.reserve(mResourceEntries.size());

        for (CResourceIterator It(this); It; ++It)
        {
            if (!It->IsTransient())
                Resources.push_back( SDatabaseResource { It->ID(), It->CookedExtension(), It->Directory()->FullPath(), It->Name() } );
        }
    }

    // Serialize
    rArc << SERIAL_CONTAINER_AUTO(Resources, "Resource");

    // Register resources
    if (rArc.IsReader())
    {
        for (auto Iter = Resources.begin(); Iter != Resources.end(); Iter++)
        {
            SDatabaseResource& rRes = *Iter;
            RegisterResource(rRes.ID, CResource::ResTypeForExtension(rRes.Type), rRes.Directory, rRes.Name);
        }
    }
}

void CResourceStore::LoadResourceDatabase()
{
    ASSERT(mpProj);
    TString Path = mpProj->ResourceDBPath(false).ToUTF8();

    CXMLReader Reader(Path);
    SerializeResourceDatabase(Reader);
    LoadCacheFile();
}

void CResourceStore::SaveResourceDatabase()
{
    ASSERT(mpProj);
    TString Path = mpProj->ResourceDBPath(false).ToUTF8();

    CXMLWriter Writer(Path, "ResourceDB", 0, mpProj ? mpProj->Game() : eUnknownGame);
    SerializeResourceDatabase(Writer);
}

void CResourceStore::LoadCacheFile()
{
    TString CacheDataPath = mpProj->ResourceCachePath(false).ToUTF8();
    CFileInStream CacheFile(CacheDataPath.ToStdString(), IOUtil::eBigEndian);
    ASSERT(CacheFile.IsValid());

    // Cache header
    CFourCC Magic(CacheFile);

    if (Magic != "CACH")
    {
        Log::Error("Invalid resource cache data magic: " + Magic.ToString());
        return;
    }

    CSerialVersion Version(CacheFile);
    u32 NumResources = CacheFile.ReadLong();

    for (u32 iRes = 0; iRes < NumResources; iRes++)
    {
        CAssetID ID(CacheFile, Version.Game());
        u32 EntryCacheSize = CacheFile.ReadLong();
        u32 EntryCacheEnd = CacheFile.Tell() + EntryCacheSize;

        CResourceEntry *pEntry = FindEntry(ID);

        if (pEntry && !pEntry->IsTransient())
        {
            CBasicBinaryReader Reader(&CacheFile, Version);

            if (Reader.ParamBegin("EntryCache"))
            {
                pEntry->SerializeCacheData(Reader);
                Reader.ParamEnd();
            }
        }

        CacheFile.Seek(EntryCacheEnd, SEEK_SET);
    }
}

void CResourceStore::SaveCacheFile()
{
    TString CacheDataPath = mpProj->ResourceCachePath(false).ToUTF8();
    CFileOutStream CacheFile(CacheDataPath.ToStdString(), IOUtil::eBigEndian);
    ASSERT(CacheFile.IsValid());

    // Cache header
    CFourCC("CACH").Write(CacheFile);
    CSerialVersion Version(0, 0, mpProj->Game());
    Version.Write(CacheFile);

    u32 ResCountOffset = CacheFile.Tell();
    u32 ResCount = 0;
    CacheFile.WriteLong(0); // Resource count dummy - fill in when we know the real count

    // Save entry cache data
    // Structure: Entry Asset ID -> Entry Cache Size -> Serialized Entry Cache Data
    for (CResourceIterator It(this); It; ++It)
    {
        if (!It->IsTransient())
        {
            ResCount++;
            It->ID().Write(CacheFile);
            u32 SizeOffset = CacheFile.Tell();
            CacheFile.WriteLong(0);

            CBasicBinaryWriter Writer(&CacheFile, Version.FileVersion(), Version.Game());

            if (Writer.ParamBegin("EntryCache"))
            {
                It->SerializeCacheData(Writer);
                Writer.ParamEnd();
            }

            u32 EntryCacheEnd = CacheFile.Tell();
            CacheFile.Seek(SizeOffset, SEEK_SET);
            CacheFile.WriteLong(EntryCacheEnd - SizeOffset - 4);
            CacheFile.Seek(EntryCacheEnd, SEEK_SET);
        }
    }

    CacheFile.Seek(ResCountOffset, SEEK_SET);
    CacheFile.WriteLong(ResCount);
}

void CResourceStore::SetActiveProject(CGameProject *pProj)
{
    if (mpProj == pProj) return;

    CloseActiveProject();
    mpProj = pProj;

    if (pProj)
    {
        mpProjectRoot = new CVirtualDirectory();

        if (!mpExporter)
            LoadResourceDatabase();
    }
}

void CResourceStore::CloseActiveProject()
{
    // Destroy unreferenced resources first. (This is necessary to avoid invalid memory accesses when
    // various TResPtrs are destroyed. There might be a cleaner solution than this.)
    DestroyUnreferencedResources();

    // Delete all entries from old project
    auto It = mResourceEntries.begin();

    while (It != mResourceEntries.end())
    {
        CResourceEntry *pEntry = It->second;

        if (!pEntry->IsTransient())
        {
            if (pEntry->IsLoaded())
            {
                bool UnloadSuccess = pEntry->Unload();
                ASSERT(UnloadSuccess);

                auto LoadIt = mLoadedResources.find(pEntry->ID());
                ASSERT(LoadIt != mLoadedResources.end());
                mLoadedResources.erase(LoadIt);
            }

            delete pEntry;
            It = mResourceEntries.erase(It);
        }

        else
            It++;
    }

    delete mpProjectRoot;
    mpProjectRoot = nullptr;
    mpProj = nullptr;
}

CVirtualDirectory* CResourceStore::GetVirtualDirectory(const TWideString& rkPath, bool Transient, bool AllowCreate)
{
    if (rkPath.IsEmpty()) return nullptr;

    else if (Transient)
    {
        for (u32 iTrans = 0; iTrans < mTransientRoots.size(); iTrans++)
        {
            if (mTransientRoots[iTrans]->Name() == rkPath)
                return mTransientRoots[iTrans];
        }

        if (AllowCreate)
        {
            CVirtualDirectory *pDir = new CVirtualDirectory(rkPath);
            mTransientRoots.push_back(pDir);
            return pDir;
        }

        else return nullptr;
    }

    else if (mpProjectRoot)
    {
        return mpProjectRoot->FindChildDirectory(rkPath, AllowCreate);
    }

    else return nullptr;
}

CResourceEntry* CResourceStore::FindEntry(const CAssetID& rkID) const
{
    if (!rkID.IsValid()) return nullptr;
    auto Found = mResourceEntries.find(rkID);
    if (Found == mResourceEntries.end()) return nullptr;
    else return Found->second;
}

bool CResourceStore::IsResourceRegistered(const CAssetID& rkID) const
{
    return FindEntry(rkID) == nullptr;
}

CResourceEntry* CResourceStore::RegisterResource(const CAssetID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkFileName)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
    {
        if (pEntry->IsTransient())
        {
            ASSERT(pEntry->ResourceType() == Type);
            pEntry->AddToProject(rkDir, rkFileName);
        }

        else
            Log::Error("Attempted to register resource that's already tracked in the database: " + rkID.ToString() + " / " + rkDir.ToUTF8() + " / " + rkFileName.ToUTF8());
    }

    else
    {
        pEntry = new CResourceEntry(this, rkID, rkDir, rkFileName.GetFileName(false), Type);
        mResourceEntries[rkID] = pEntry;
    }

    return pEntry;
}

CResourceEntry* CResourceStore::RegisterTransientResource(EResType Type, const TWideString& rkDir /*= L""*/, const TWideString& rkFileName /*= L""*/)
{
    CResourceEntry *pEntry = new CResourceEntry(this, CAssetID::RandomID(), rkDir, rkFileName, Type, true);
    mResourceEntries[pEntry->ID()] = pEntry;
    return pEntry;
}

CResourceEntry* CResourceStore::RegisterTransientResource(EResType Type, const CAssetID& rkID, const TWideString& rkDir /*=L ""*/, const TWideString& rkFileName /*= L""*/)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (!pEntry)
    {
        pEntry = new CResourceEntry(this, rkID, rkDir, rkFileName, Type, true);
        mResourceEntries[rkID] = pEntry;
    }

    return pEntry;
}

CResource* CResourceStore::LoadResource(const CAssetID& rkID, const CFourCC& rkType)
{
    if (!rkID.IsValid()) return nullptr;

    // Check if resource is already loaded
    auto Find = mLoadedResources.find(rkID);
    if (Find != mLoadedResources.end())
        return Find->second->Resource();

    // With Game Exporter - Get data buffer from exporter
    if (mpExporter)
    {
        std::vector<u8> DataBuffer;
        mpExporter->LoadResource(rkID, DataBuffer);
        if (DataBuffer.empty()) return nullptr;

        CMemoryInStream MemStream(DataBuffer.data(), DataBuffer.size(), IOUtil::eBigEndian);
        EResType Type = CResource::ResTypeForExtension(rkType);
        CResourceEntry *pEntry = RegisterTransientResource(Type, rkID);
        CResource *pRes = pEntry->LoadCooked(MemStream);
        return pRes;
    }

    // Without Game Exporter - Check store resource entries and transient load directory.
    else
    {
        // Check for resource in store
        CResourceEntry *pEntry = FindEntry(rkID);
        if (pEntry) return pEntry->Load();

        // Check in transient load directory - this only works for cooked
        EResType Type = CResource::ResTypeForExtension(rkType);

        if (Type != eInvalidResType)
        {
            // Note the entry may not be able to find the resource on its own (due to not knowing what game
            // it is) so we will attempt to open the file stream ourselves and pass it to the entry instead.
            TString Name = rkID.ToString();
            CResourceEntry *pEntry = RegisterTransientResource(Type, mTransientLoadDir, Name.ToUTF16());

            TString Path = mTransientLoadDir.ToUTF8() + Name + "." + rkType.ToString();
            CFileInStream File(Path.ToStdString(), IOUtil::eBigEndian);
            CResource *pRes = pEntry->LoadCooked(File);

            if (!pRes) DeleteResourceEntry(pEntry);
            return pRes;
        }

        else
        {
            Log::Error("Can't load requested resource with ID \"" + rkID.ToString() + "\"; can't locate resource. Note: Loading raw assets from an arbitrary directory is unsupported.");;
            return nullptr;
        }
    }
}

CResource* CResourceStore::LoadResource(const TString& rkPath)
{
    // todo - support loading raw resources from arbitrary directory
    // Construct ID from string, check if resource is loaded already
    TWideString Dir = FileUtil::MakeAbsolute(TWideString(rkPath.GetFileDirectory()));
    TString Name = rkPath.GetFileName(false);
    CAssetID ID = (Name.IsHexString() ? Name.ToInt64() : rkPath.Hash64());
    auto Find = mLoadedResources.find(ID);

    if (Find != mLoadedResources.end())
        return Find->second->Resource();

    // Determine type
    TString Extension = rkPath.GetFileExtension().ToUpper();
    EResType Type = CResource::ResTypeForExtension(Extension);

    if (Type == eInvalidResType)
    {
        Log::Error("Unable to load resource " + rkPath + "; unrecognized extension: " + Extension);
        return nullptr;
    }

    // Open file
    CFileInStream File(rkPath.ToStdString(), IOUtil::eBigEndian);

    if (!File.IsValid())
    {
        Log::Error("Unable to load resource; couldn't open file: " + rkPath);
        return nullptr;
    }

    // Load resource
    TString OldTransientDir = mTransientLoadDir;
    mTransientLoadDir = Dir;

    CResourceEntry *pEntry = RegisterTransientResource(Type, ID, Dir, Name);
    CResource *pRes = pEntry->LoadCooked(File);
    if (!pRes) DeleteResourceEntry(pEntry);

    mTransientLoadDir = OldTransientDir;

    return pRes;
}

void CResourceStore::TrackLoadedResource(CResourceEntry *pEntry)
{
    ASSERT(pEntry->IsLoaded());
    ASSERT(mLoadedResources.find(pEntry->ID()) == mLoadedResources.end());
    mLoadedResources[pEntry->ID()] = pEntry;
}

CFourCC CResourceStore::ResourceTypeByID(const CAssetID& rkID, const TStringList& rkPossibleTypes) const
{
    if (!rkID.IsValid()) return eInvalidResType;
    if (rkPossibleTypes.size() == 1) return CFourCC(rkPossibleTypes.front());

    // Check for existing entry
    auto Find = mResourceEntries.find(rkID);
    if (Find != mResourceEntries.end())
        return GetResourceCookedExtension(Find->second->ResourceType(), Find->second->Game());

    // Determine extension from filesystem - try every extension until we find the file
    TString PathBase = mTransientLoadDir.ToUTF8() + rkID.ToString() + '.';

    for (auto It = rkPossibleTypes.begin(); It != rkPossibleTypes.end(); It++)
    {
        TString NewPath = PathBase + *It;

        if (FileUtil::Exists(NewPath))
            return CFourCC(*It);
    }

    // Couldn't find one, so return unknown. Note that it'd be possible to look up the extension from the
    // filesystem even if it's not one of the provided possible types, but this would be too slow.
    return "UNKN";
}

void CResourceStore::DestroyUnreferencedResources()
{
    // This can be updated to avoid the do-while loop when reference lookup is implemented.
    u32 NumDeleted;

    do
    {
        NumDeleted = 0;
        auto It = mLoadedResources.begin();

        while (It != mLoadedResources.end())
        {
            CResourceEntry *pEntry = It->second;

            if (!pEntry->Resource()->IsReferenced() && pEntry->Unload())
            {
                It = mLoadedResources.erase(It);
                NumDeleted++;

                // Transient resources should have their entries cleared out when the resource is unloaded
                if (pEntry->IsTransient())
                    DeleteResourceEntry(pEntry);
            }

            else It++;
        }
    } while (NumDeleted > 0);

    // Destroy empty virtual directories
    for (auto DirIt = mTransientRoots.begin(); DirIt != mTransientRoots.end(); DirIt++)
    {
        CVirtualDirectory *pRoot = *DirIt;

        if (pRoot->IsEmpty())
        {
            delete pRoot;
            DirIt = mTransientRoots.erase(DirIt);
        }
    }
}

bool CResourceStore::DeleteResourceEntry(CResourceEntry *pEntry)
{
    CAssetID ID = pEntry->ID();

    if (pEntry->IsLoaded())
    {
        if (!pEntry->Unload())
            return false;

        auto It = mLoadedResources.find(ID);
        ASSERT(It != mLoadedResources.end());
        mLoadedResources.erase(It);
    }

    auto It = mResourceEntries.find(ID);
    ASSERT(It != mResourceEntries.end());
    mResourceEntries.erase(It);

    delete pEntry;
    return true;
}

void CResourceStore::SetTransientLoadDir(const TString& rkDir)
{
    mTransientLoadDir = rkDir;
    mTransientLoadDir.EnsureEndsWith('\\');
    Log::Write("Set resource directory: " + rkDir);
}
