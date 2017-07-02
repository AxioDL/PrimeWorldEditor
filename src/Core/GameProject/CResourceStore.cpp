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

// Constructor for editor store
CResourceStore::CResourceStore(const TString& rkDatabasePath)
    : mpProj(nullptr)
    , mGame(eUnknownGame)
    , mDatabaseDirty(false)
    , mCacheFileDirty(false)
{
    mpDatabaseRoot = new CVirtualDirectory(this);
    mDatabasePath = FileUtil::MakeAbsolute(rkDatabasePath.GetFileDirectory());
    mDatabaseName = rkDatabasePath.GetFileName();
}

// Main constructor for game projects and game exporter
CResourceStore::CResourceStore(CGameProject *pProject)
    : mpProj(nullptr)
    , mGame(eUnknownGame)
    , mpDatabaseRoot(nullptr)
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
}

void CResourceStore::SerializeResourceDatabase(IArchive& rArc)
{
    struct SDatabaseResource
    {
        CAssetID ID;
        CResTypeInfo *pType;
        TString Directory;
        TString Name;

        void Serialize(IArchive& rArc)
        {
            rArc << SERIAL_AUTO(ID) << SERIAL("Type", pType) << SERIAL_AUTO(Directory) << SERIAL_AUTO(Name);
        }
    };
    std::vector<SDatabaseResource> Resources;

    // Populate resource list
    if (!rArc.IsReader())
    {
        Resources.reserve(mResourceEntries.size());

        for (CResourceIterator It(this); It; ++It)
            Resources.push_back( SDatabaseResource { It->ID(), It->TypeInfo(), It->Directory()->FullPath(), It->Name() } );
    }

    // Serialize
    rArc << SERIAL_CONTAINER_AUTO(Resources, "Resource");

    // Register resources
    if (rArc.IsReader())
    {
        for (auto Iter = Resources.begin(); Iter != Resources.end(); Iter++)
        {
            SDatabaseResource& rRes = *Iter;
            RegisterResource(rRes.ID, rRes.pType->Type(), rRes.Directory, rRes.Name);
        }
    }
}

bool CResourceStore::LoadResourceDatabase()
{
    ASSERT(!mDatabasePath.IsEmpty());
    TString Path = DatabasePath();

    if (!mpDatabaseRoot)
        mpDatabaseRoot = new CVirtualDirectory(this);

    CXMLReader Reader(Path);

    if (!Reader.IsValid())
    {
        Log::Error("Failed to open resource database for load: " + Path);
        return false;
    }

    if (mpProj)
        ASSERT(mpProj->Game() == Reader.Game());

    mGame = Reader.Game();
    SerializeResourceDatabase(Reader);
    return LoadCacheFile();
}

bool CResourceStore::SaveResourceDatabase()
{
    TString Path = DatabasePath();
    CXMLWriter Writer(Path, "ResourceDB", 0, mGame);
    SerializeResourceDatabase(Writer);
    bool SaveSuccess = Writer.Save();

    if (SaveSuccess)
        mDatabaseDirty = false;
    else
        Log::Error("Failed to save resource database: " + Path);

    return SaveSuccess;
}

bool CResourceStore::LoadCacheFile()
{
    TString CachePath = CacheDataPath();
    CFileInStream CacheFile(CachePath, IOUtil::eBigEndian);

    if (!CacheFile.IsValid())
    {
        Log::Error("Failed to open cache file for load: " + CachePath);
        return false;
    }

    // Cache header
    CFourCC Magic(CacheFile);

    if (Magic != FOURCC('CACH'))
    {
        Log::Error("Invalid resource cache data magic: " + Magic.ToString());
        return false;
    }

    CSerialVersion Version(CacheFile);
    u32 NumResources = CacheFile.ReadLong();

    for (u32 iRes = 0; iRes < NumResources; iRes++)
    {
        CAssetID ID(CacheFile, Version.Game());
        u32 EntryCacheSize = CacheFile.ReadLong();
        u32 EntryCacheEnd = CacheFile.Tell() + EntryCacheSize;

        CResourceEntry *pEntry = FindEntry(ID);

        if (pEntry)
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
    return true;
}

bool CResourceStore::SaveCacheFile()
{
    TString CachePath = CacheDataPath();
    CFileOutStream CacheFile(CachePath, IOUtil::eBigEndian);

    if (!CacheFile.IsValid())
    {
        Log::Error("Failed to open cache file for save: " + CachePath);
        return false;
    }

    // Cache header
    CacheFile.WriteLong(0); // Magic dummy. Magic isn't written until the rest of the file is saved successfully.
    CSerialVersion Version(IArchive::skCurrentArchiveVersion, 0, mGame);
    Version.Write(CacheFile);

    u32 ResCountOffset = CacheFile.Tell();
    u32 ResCount = 0;
    CacheFile.WriteLong(0); // Resource count dummy - fill in when we know the real count

    // Save entry cache data
    // Structure: Entry Asset ID -> Entry Cache Size -> Serialized Entry Cache Data
    for (CResourceIterator It(this); It; ++It)
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

    CacheFile.Seek(ResCountOffset, SEEK_SET);
    CacheFile.WriteLong(ResCount);
    CacheFile.Seek(0, SEEK_SET);
    CacheFile.WriteLong( FOURCC('CACH') );
    mCacheFileDirty = false;
    return true;
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
        TString DatabasePath = mpProj->ResourceDBPath(false);
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

    // There should be no loaded resources!!!
    // If there are, that means something didn't clean up resource references properly on project close!!!
    if (!mLoadedResources.empty())
    {
        Log::Error(TString::FromInt32(mLoadedResources.size(), 0, 10) + " resources still loaded on project close:");

        for (auto Iter = mLoadedResources.begin(); Iter != mLoadedResources.end(); Iter++)
        {
            CResourceEntry *pEntry = Iter->second;
            Log::Write("\t" + pEntry->Name() + "." + pEntry->CookedExtension().ToString());
        }

        ASSERT(false);
    }

    // Delete all entries from old project
    auto It = mResourceEntries.begin();

    while (It != mResourceEntries.end())
    {
        delete It->second;
        It = mResourceEntries.erase(It);
    }

    delete mpDatabaseRoot;
    mpDatabaseRoot = nullptr;
    mpProj = nullptr;
    mGame = eUnknownGame;
}

CVirtualDirectory* CResourceStore::GetVirtualDirectory(const TString& rkPath, bool AllowCreate)
{
    if (rkPath.IsEmpty())
        return mpDatabaseRoot;
    else if (mpDatabaseRoot)
        return mpDatabaseRoot->FindChildDirectory(rkPath, AllowCreate);
    else
        return nullptr;
}

void CResourceStore::ConditionalDeleteDirectory(CVirtualDirectory *pDir)
{
    if (pDir->IsEmpty())
    {
        // If this directory is part of the project, then we should delete the corresponding filesystem directory
        if (pDir->GetRoot() == mpDatabaseRoot && !pDir->IsRoot())
        {
            FileUtil::DeleteDirectory(ResourcesDir() + pDir->FullPath(), true);
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

CResourceEntry* CResourceStore::FindEntry(const TString& rkPath) const
{
    return (mpDatabaseRoot ? mpDatabaseRoot->FindChildResource(rkPath) : nullptr);
}

bool CResourceStore::IsResourceRegistered(const CAssetID& rkID) const
{
    return FindEntry(rkID) != nullptr;
}

CResourceEntry* CResourceStore::RegisterResource(const CAssetID& rkID, EResType Type, const TString& rkDir, const TString& rkName)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
        Log::Error("Attempted to register resource that's already tracked in the database: " + rkID.ToString() + " / " + rkDir + " / " + rkName);

    else
    {
        // Validate directory
        if (IsValidResourcePath(rkDir, rkName))
        {
            pEntry = new CResourceEntry(this, rkID, rkDir, rkName, Type);
            pEntry->LoadMetadata();
            mResourceEntries[rkID] = pEntry;
        }

        else
            Log::Error("Invalid resource path, failed to register: " + rkDir + rkName);
    }

    return pEntry;
}

CResource* CResourceStore::LoadResource(const CAssetID& rkID)
{
    if (!rkID.IsValid()) return nullptr;

    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
        return pEntry->Load();

    else
    {
        // Resource doesn't seem to exist
        Log::Error("Can't find requested resource with ID \"" + rkID.ToString() + "\".");;
        return nullptr;
    }
}

CResource* CResourceStore::LoadResource(const CAssetID& rkID, EResType Type)
{
    CResource *pRes = LoadResource(rkID);

    if (pRes)
    {
        if (pRes->Type() == Type)
        {
            return pRes;
        }
        else
        {
            CResTypeInfo *pExpectedType = CResTypeInfo::FindTypeInfo(Type);
            CResTypeInfo *pGotType = pRes->TypeInfo();
            ASSERT(pExpectedType && pGotType);

            Log::Error("Resource with ID \"" + rkID.ToString() + "\" requested with the wrong type; expected " + pExpectedType->TypeName() + " asset, got " + pGotType->TypeName() + " asset");
            return nullptr;
        }
    }

    else
        return nullptr;
}

CResource* CResourceStore::LoadResource(const TString& rkPath)
{
    // If this is a relative path then load via the resource DB
    CResourceEntry *pEntry = FindEntry(rkPath);

    if (pEntry)
    {
        // Verify extension matches the entry + load resource
        TString Ext = rkPath.GetFileExtension();

        if (!Ext.IsEmpty())
        {
            if (Ext.Length() == 4)
            {
                ASSERT( Ext.CaseInsensitiveCompare(pEntry->CookedExtension().ToString()) );
            }
            else
            {
                ASSERT( rkPath.EndsWith(pEntry->RawExtension()) );
            }
        }

        return pEntry->Load();
    }

    else return nullptr;
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
            }

            else It++;
        }
    } while (NumDeleted > 0);
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

    if (pEntry->Directory())
        pEntry->Directory()->RemoveChildResource(pEntry);

    auto It = mResourceEntries.find(ID);
    ASSERT(It != mResourceEntries.end());
    mResourceEntries.erase(It);

    delete pEntry;
    return true;
}

void CResourceStore::ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly)
{
    // Read file contents -first- then move assets -after-; this
    // 1. avoids anything fucking up if the contents file is badly formatted and we crash, and
    // 2. avoids extra redundant moves (since there are redundant entries in the file)
    // todo: move to CAssetNameMap?
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
        u32 PathEnd = Line.Size() - 5;

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

            // If the "x_rep" folder doesn't exist in this path for some reason, but this is still a path, then just chop off the drive letter.
            // Otherwise, this is most likely just a standalone name, so use the full name as-is.
            else if (Path[1] == ':')
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

        TString Path = Iter->second;
        TString Dir = Path.GetFileDirectory();
        TString Name = Path.GetFileName(false);
        if (Dir.IsEmpty()) Dir = pEntry->DirectoryPath();

        pEntry->Move(Dir, Name);
    }

    // Save
    ConditionalSaveStore();
}

bool CResourceStore::IsValidResourcePath(const TString& rkPath, const TString& rkName)
{
    // Path must not be an absolute path and must not go outside the project structure.
    // Name must not be a path.
    return ( CVirtualDirectory::IsValidDirectoryPath(rkPath) &&
             FileUtil::IsValidName(rkName, false) &&
             !rkName.Contains('/') &&
             !rkName.Contains('\\') );
}
