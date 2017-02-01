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
CResourceStore *gpResourceStore = nullptr;
CResourceStore *gpEditorStore = nullptr;

CResourceStore::CResourceStore(const TWideString& rkDatabasePath)
    : mpProj(nullptr)
    , mGame(eUnknownGame)
    , mpExporter(nullptr)
    , mDatabaseDirty(false)
    , mCacheFileDirty(false)
{
    mpDatabaseRoot = new CVirtualDirectory(this);
    mDatabasePath = FileUtil::MakeAbsolute(rkDatabasePath.GetFileDirectory());
    mDatabaseName = rkDatabasePath.GetFileName();
}

CResourceStore::CResourceStore(CGameProject *pProject, CGameExporter *pExporter, const TWideString& rkRawDir, const TWideString& rkCookedDir, EGame Game)
    : mpProj(nullptr)
    , mGame(Game)
    , mRawDir(rkRawDir)
    , mCookedDir(rkCookedDir)
    , mpExporter(pExporter)
    , mDatabaseDirty(false)
    , mCacheFileDirty(false)
{
    SetProject(pProject);
}

CResourceStore::CResourceStore(CGameProject *pProject)
    : mpProj(nullptr)
    , mGame(eUnknownGame)
    , mpDatabaseRoot(nullptr)
    , mpExporter(nullptr)
    , mDatabaseDirty(false)
    , mCacheFileDirty(false)
{
    SetProject(pProject);
}

CResourceStore::~CResourceStore()
{
    CloseProject();
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
    rArc << SERIAL("RawDir", mRawDir)
         << SERIAL("CookedDir", mCookedDir)
         << SERIAL_CONTAINER_AUTO(Resources, "Resource");

    // Register resources
    if (rArc.IsReader())
    {
        for (auto Iter = Resources.begin(); Iter != Resources.end(); Iter++)
        {
            SDatabaseResource& rRes = *Iter;
            RegisterResource(rRes.ID, CResTypeInfo::TypeForCookedExtension(rArc.Game(), rRes.Type)->Type(), rRes.Directory, rRes.Name);
        }
    }
}

void CResourceStore::LoadResourceDatabase()
{
    ASSERT(!mDatabasePath.IsEmpty());
    TString Path = DatabasePath().ToUTF8();

    if (!mpDatabaseRoot)
        mpDatabaseRoot = new CVirtualDirectory(this);

    CXMLReader Reader(Path);

    if (mpProj)
        ASSERT(mpProj->Game() == Reader.Game());

    mGame = Reader.Game();
    SerializeResourceDatabase(Reader);
    LoadCacheFile();
}

void CResourceStore::SaveResourceDatabase()
{
    TString Path = DatabasePath().ToUTF8();
    CXMLWriter Writer(Path, "ResourceDB", 0, mGame);
    SerializeResourceDatabase(Writer);
    mDatabaseDirty = false;
}

void CResourceStore::LoadCacheFile()
{
    TString CachePath = CacheDataPath().ToUTF8();
    CFileInStream CacheFile(CachePath.ToStdString(), IOUtil::eBigEndian);
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
    TString CachePath = CacheDataPath().ToUTF8();
    CFileOutStream CacheFile(CachePath.ToStdString(), IOUtil::eBigEndian);
    ASSERT(CacheFile.IsValid());

    // Cache header
    CFourCC("CACH").Write(CacheFile);
    CSerialVersion Version(0, 0, mGame);
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
    mCacheFileDirty = false;
}

void CResourceStore::ConditionalSaveStore()
{
    if (mDatabaseDirty)  SaveResourceDatabase();
    if (mCacheFileDirty) SaveCacheFile();
}

void CResourceStore::SetProject(CGameProject *pProj)
{
    if (mpProj == pProj) return;

    if (mpProj)
        CloseProject();

    mpProj = pProj;

    if (mpProj)
    {
        TWideString DatabasePath = mpProj->ResourceDBPath(false);
        mDatabasePath = DatabasePath.GetFileDirectory();
        mDatabaseName = DatabasePath.GetFileName();
        mpDatabaseRoot = new CVirtualDirectory(this);
        mGame = mpProj->Game();
    }
}

void CResourceStore::CloseProject()
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

    delete mpDatabaseRoot;
    mpDatabaseRoot = nullptr;
    mpProj = nullptr;
    mGame = eUnknownGame;
}

CVirtualDirectory* CResourceStore::GetVirtualDirectory(const TWideString& rkPath, bool Transient, bool AllowCreate)
{
    if (rkPath.IsEmpty()) return mpDatabaseRoot;

    else if (Transient)
    {
        for (u32 iTrans = 0; iTrans < mTransientRoots.size(); iTrans++)
        {
            if (mTransientRoots[iTrans]->Name() == rkPath)
                return mTransientRoots[iTrans];
        }

        if (AllowCreate)
        {
            CVirtualDirectory *pDir = new CVirtualDirectory(rkPath, this);
            mTransientRoots.push_back(pDir);
            return pDir;
        }

        else return nullptr;
    }

    else if (mpDatabaseRoot)
    {
        return mpDatabaseRoot->FindChildDirectory(rkPath, AllowCreate);
    }

    else return nullptr;
}

void CResourceStore::ConditionalDeleteDirectory(CVirtualDirectory *pDir)
{
    if (pDir->IsEmpty())
    {
        // If this directory is part of the project, then we should delete the corresponding filesystem directories
        if (pDir->GetRoot() == mpDatabaseRoot)
        {
            FileUtil::DeleteDirectory(RawDir(false) + pDir->FullPath());
            FileUtil::DeleteDirectory(CookedDir(false) + pDir->FullPath());
        }

        CVirtualDirectory *pParent = pDir->Parent();
        pParent->RemoveChildDirectory(pDir);
        ConditionalDeleteDirectory(pParent);
    }
}

CResourceEntry* CResourceStore::FindEntry(const CAssetID& rkID) const
{
    if (!rkID.IsValid()) return nullptr;
    auto Found = mResourceEntries.find(rkID);
    if (Found == mResourceEntries.end()) return nullptr;
    else return Found->second;
}

CResourceEntry* CResourceStore::FindEntry(const TWideString& rkPath) const
{
    return (mpDatabaseRoot ? mpDatabaseRoot->FindChildResource(rkPath) : nullptr);
}

bool CResourceStore::IsResourceRegistered(const CAssetID& rkID) const
{
    return FindEntry(rkID) != nullptr;
}

CResourceEntry* CResourceStore::RegisterResource(const CAssetID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkName)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
    {
        if (pEntry->IsTransient())
        {
            ASSERT(pEntry->ResourceType() == Type);
            pEntry->AddToProject(rkDir, rkName);
        }

        else
            Log::Error("Attempted to register resource that's already tracked in the database: " + rkID.ToString() + " / " + rkDir.ToUTF8() + " / " + rkName.ToUTF8());
    }

    else
    {
        pEntry = new CResourceEntry(this, rkID, rkDir, rkName, Type);
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
        EResType Type = CResTypeInfo::TypeForCookedExtension(mGame, rkType)->Type();
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
        EResType Type = CResTypeInfo::TypeForCookedExtension(mGame, rkType)->Type();

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

CResource* CResourceStore::LoadResource(const TWideString& rkPath)
{
    // If this is a relative path then load via the resource DB
    if (!FileUtil::IsAbsolute(rkPath))
    {
        CResourceEntry *pEntry = FindEntry(rkPath);

        if (pEntry)
        {
            // Verify extension matches the entry + load resource
            TString Ext = rkPath.ToUTF8().GetFileExtension();

            if (!Ext.IsEmpty())
            {
                if (Ext.Length() == 4)
                {
                    ASSERT(Ext.CaseInsensitiveCompare(pEntry->CookedExtension().ToString()));
                }
                else
                {
                    ASSERT(Ext.CaseInsensitiveCompare(pEntry->RawExtension()));
                }
            }

            return pEntry->Load();
        }

        else return nullptr;
    }

    // Otherwise create transient entry; construct ID from string, check if resource is loaded already
    TWideString Dir = FileUtil::MakeAbsolute(TWideString(rkPath.GetFileDirectory()));
    TString Name = rkPath.GetFileName(false);
    CAssetID ID = (Name.IsHexString() ? Name.ToInt64() : rkPath.Hash64());
    auto Find = mLoadedResources.find(ID);

    if (Find != mLoadedResources.end())
        return Find->second->Resource();

    // Determine type
    TString PathUTF8 = rkPath.ToUTF8();
    TString Extension = TString(PathUTF8).GetFileExtension().ToUpper();
    EResType Type = CResTypeInfo::TypeForCookedExtension(mGame, Extension)->Type();

    if (Type == eInvalidResType)
    {
        Log::Error("Unable to load resource " + PathUTF8 + "; unrecognized extension: " + Extension);
        return nullptr;
    }

    // Open file
    CFileInStream File(PathUTF8.ToStdString(), IOUtil::eBigEndian);

    if (!File.IsValid())
    {
        Log::Error("Unable to load resource; couldn't open file: " + PathUTF8);
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

void CResourceStore::ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly)
{
    // Read file contents -first- then move assets -after-; this
    // 1. avoids anything fucking up if the contents file is badly formatted and we crash, and
    // 2. avoids extra redundant moves (since there are redundant entries in the file)
    std::map<CResourceEntry*, TString> PathMap;
    FILE *pContentsFile;
    fopen_s(&pContentsFile, *rkTxtPath, "r");

    if (!pContentsFile)
    {
        Log::Error("Failed to open .contents.txt file: " + rkTxtPath);
        return;
    }

    while (!feof(pContentsFile))
    {
        // Get new line, parse to extract the ID/path
        char LineBuffer[512];
        fgets(LineBuffer, 512, pContentsFile);

        TString Line(LineBuffer);
        if (Line.IsEmpty()) break;

        u32 IDStart = Line.IndexOfPhrase("0x") + 2;
        if (IDStart == 1) continue;

        u32 IDEnd = Line.IndexOf(" \t", IDStart);
        u32 PathStart = IDEnd + 1;
        u32 PathEnd = Line.Size() - 4;

        TString IDStr = Line.SubString(IDStart, IDEnd - IDStart);
        TString Path = Line.SubString(PathStart, PathEnd - PathStart);

        CAssetID ID = CAssetID::FromString(IDStr);
        CResourceEntry *pEntry = FindEntry(ID);

        // Only process this entry if the ID exists
        if (pEntry)
        {
            // Chop name to just after "x_rep"
            u32 RepStart = Path.IndexOfPhrase("_rep");

            if (RepStart != -1)
                Path = Path.ChopFront(RepStart + 5);

            // If the "x_rep" folder doesn't exist in this path for some reason, then just chop off the drive letter
            else
                Path = Path.ChopFront(3);

            PathMap[pEntry] = Path;
        }
    }

    fclose(pContentsFile);

    // Assign names
    for (auto Iter = PathMap.begin(); Iter != PathMap.end(); Iter++)
    {
        CResourceEntry *pEntry = Iter->first;
        if (UnnamedOnly && pEntry->IsNamed()) continue;

        TWideString Path = Iter->second.ToUTF16();
        pEntry->Move(Path.GetFileDirectory(), Path.GetFileName(false));
    }

    // Save
    ConditionalSaveStore();
}
