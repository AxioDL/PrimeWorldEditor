#include "CResourceStore.h"
#include "CGameExporter.h"
#include "CGameProject.h"
#include "CResourceIterator.h"
#include "Core/IUIRelay.h"
#include "Core/Resource/CResource.h"
#include <Common/Macros.h>
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
    , mGame(EGame::Prime)
    , mDatabaseCacheDirty(false)
{
    mpDatabaseRoot = new CVirtualDirectory(this);
    mDatabasePath = FileUtil::MakeAbsolute(rkDatabasePath.GetFileDirectory());
    LoadDatabaseCache();
}

// Main constructor for game projects and game exporter
CResourceStore::CResourceStore(CGameProject *pProject)
    : mpProj(nullptr)
    , mGame(EGame::Invalid)
    , mpDatabaseRoot(nullptr)
    , mDatabaseCacheDirty(false)
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

void RecursiveGetListOfEmptyDirectories(CVirtualDirectory *pDir, TStringList& rOutList)
{
    // Helper function for SerializeResourceDatabase
    if (pDir->IsEmpty(false))
    {
        rOutList.push_back(pDir->FullPath());
    }
    else
    {
        for (uint32 SubIdx = 0; SubIdx < pDir->NumSubdirectories(); SubIdx++)
            RecursiveGetListOfEmptyDirectories(pDir->SubdirectoryByIndex(SubIdx), rOutList);
    }
}

bool CResourceStore::SerializeDatabaseCache(IArchive& rArc)
{
    // Serialize resources
    if (rArc.ParamBegin("Resources", 0))
    {
        // Serialize resources
        uint32 ResourceCount = mResourceEntries.size();

        if (rArc.IsWriter())
        {
            // Make sure deleted resources aren't included in the count.
            // We can't use CResourceIterator because it skips MarkedForDeletion resources.
            for (auto Iter = mResourceEntries.begin(); Iter != mResourceEntries.end(); Iter++)
            {
                CResourceEntry* pEntry = Iter->second;

                if (pEntry->IsMarkedForDeletion())
                {
                    ResourceCount--;
                }
            }
        }

        rArc << SerialParameter("ResourceCount", ResourceCount);

        if (rArc.IsReader())
        {
            for (uint32 ResIdx = 0; ResIdx < ResourceCount; ResIdx++)
            {
                if (rArc.ParamBegin("Resource", 0))
                {
                    CResourceEntry *pEntry = CResourceEntry::BuildFromArchive(this, rArc);
                    ASSERT( FindEntry(pEntry->ID()) == nullptr );
                    mResourceEntries[pEntry->ID()] = pEntry;
                    rArc.ParamEnd();
                }
            }
        }
        else
        {
            for (CResourceIterator It(this); It; ++It)
            {
                if (!It->IsMarkedForDeletion())
                {
                    if (rArc.ParamBegin("Resource", 0))
                    {
                        It->SerializeEntryInfo(rArc, false);
                        rArc.ParamEnd();
                    }
                }
            }
        }
        rArc.ParamEnd();
    }

    // Serialize empty directory list
    TStringList EmptyDirectories;

    if (!rArc.IsReader())
        RecursiveGetListOfEmptyDirectories(mpDatabaseRoot, EmptyDirectories);

    rArc << SerialParameter("EmptyDirectories", EmptyDirectories);

    if (rArc.IsReader())
    {
        for (auto Iter = EmptyDirectories.begin(); Iter != EmptyDirectories.end(); Iter++)
        {
            // Don't create empty virtual directories that don't actually exist in the filesystem
            TString AbsPath = ResourcesDir() + *Iter;

            if (FileUtil::Exists(AbsPath))
                CreateVirtualDirectory(*Iter);
        }
    }

    return true;
}

bool CResourceStore::LoadDatabaseCache()
{
    ASSERT(!mDatabasePath.IsEmpty());
    TString Path = DatabasePath();

    if (!mpDatabaseRoot)
        mpDatabaseRoot = new CVirtualDirectory(this);

    // Load the resource database
    CBasicBinaryReader Reader(Path, FOURCC('CACH'));

    if (!Reader.IsValid() || !SerializeDatabaseCache(Reader))
    {
        if (gpUIRelay->AskYesNoQuestion("Error", "Failed to load the resource database. Attempt to build from the directory? (This may take a while.)"))
        {
            if (!BuildFromDirectory(true))
                return false;
        }
        else return false;
    }
    else
    {
        // Database is successfully loaded at this point
        if (mpProj)
        {
            ASSERT(mpProj->Game() == Reader.Game());
        }

        mGame = Reader.Game();
    }

    return true;
}

bool CResourceStore::SaveDatabaseCache()
{
    TString Path = DatabasePath();
    debugf("Saving database cache...");

    CBasicBinaryWriter Writer(Path, FOURCC('CACH'), 0, mGame);

    if (!Writer.IsValid())
        return false;

    SerializeDatabaseCache(Writer);
    mDatabaseCacheDirty = false;
    return true;
}

void CResourceStore::ConditionalSaveStore()
{
    if (mDatabaseCacheDirty) SaveDatabaseCache();
}

void CResourceStore::SetProject(CGameProject *pProj)
{
    if (mpProj == pProj) return;

    if (mpProj)
        CloseProject();

    mpProj = pProj;

    if (mpProj)
    {
        mDatabasePath = mpProj->ProjectRoot();
        mpDatabaseRoot = new CVirtualDirectory(this);
        mGame = mpProj->Game();

        // Clear deleted files from previous runs
        TString DeletedPath = DeletedResourcePath();

        if (FileUtil::Exists(DeletedPath))
        {
            FileUtil::ClearDirectory(DeletedPath);
        }
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
        warnf("%d resources still loaded on project close:", mLoadedResources.size());

        for (auto Iter = mLoadedResources.begin(); Iter != mLoadedResources.end(); Iter++)
        {
            CResourceEntry *pEntry = Iter->second;
            warnf("\t%s.%s", *pEntry->Name(), *pEntry->CookedExtension().ToString());
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

    // Clear deleted files from previous runs
    TString DeletedPath = DeletedResourcePath();

    if (FileUtil::Exists(DeletedPath))
    {
        FileUtil::ClearDirectory(DeletedPath);
    }

    delete mpDatabaseRoot;
    mpDatabaseRoot = nullptr;
    mpProj = nullptr;
    mGame = EGame::Invalid;
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

void CResourceStore::CreateVirtualDirectory(const TString& rkPath)
{
    if (!rkPath.IsEmpty())
        mpDatabaseRoot->FindChildDirectory(rkPath, true);
}

void CResourceStore::ConditionalDeleteDirectory(CVirtualDirectory *pDir, bool Recurse)
{
    if (pDir->IsEmpty(true) && !pDir->IsRoot())
    {
        CVirtualDirectory *pParent = pDir->Parent();
        pParent->RemoveChildDirectory(pDir);

        if (Recurse)
        {
            ConditionalDeleteDirectory(pParent, true);
        }
    }
}

TString CResourceStore::DefaultResourceDirPath() const
{
    return StaticDefaultResourceDirPath( mGame );
}

TString CResourceStore::DeletedResourcePath() const
{
    return mpProj->HiddenFilesDir() / "delete/";
}

CResourceEntry* CResourceStore::FindEntry(const CAssetID& rkID) const
{
    if (rkID.IsValid())
    {
        auto Found = mResourceEntries.find(rkID);

        if (Found != mResourceEntries.end())
        {
            CResourceEntry* pEntry = Found->second;

            if (!pEntry->IsMarkedForDeletion())
                return pEntry;
        }
    }

    return nullptr;
}

CResourceEntry* CResourceStore::FindEntry(const TString& rkPath) const
{
    return (mpDatabaseRoot ? mpDatabaseRoot->FindChildResource(rkPath) : nullptr);
}

bool CResourceStore::AreAllEntriesValid() const
{
    for (CResourceIterator Iter(this); Iter; ++Iter)
    {
        if (!Iter->HasCookedVersion() && !Iter->HasRawVersion())
            return false;
    }

    return true;
}

void CResourceStore::ClearDatabase()
{
    // THIS OPERATION REQUIRES THAT ALL RESOURCES ARE UNREFERENCED
    DestroyUnreferencedResources();

    if (!mLoadedResources.empty())
    {
        debugf("ERROR: Resources still loaded:");
        for (auto Iter = mLoadedResources.begin(); Iter != mLoadedResources.end(); Iter++)
            debugf("\t[%s] %s", *Iter->first.ToString(), *Iter->second->CookedAssetPath(true));
        ASSERT(false);
    }

    // Clear out existing resource entries and directories
    for (auto Iter = mResourceEntries.begin(); Iter != mResourceEntries.end(); Iter++)
        delete Iter->second;
    mResourceEntries.clear();

    delete mpDatabaseRoot;
    mpDatabaseRoot = new CVirtualDirectory(this);

    mDatabaseCacheDirty = true;
}

bool CResourceStore::BuildFromDirectory(bool ShouldGenerateCacheFile)
{
    ASSERT(mResourceEntries.empty());

    // Get list of resources
    TString ResDir = ResourcesDir();
    TStringList ResourceList;
    FileUtil::GetDirectoryContents(ResDir, ResourceList);

    for (auto Iter = ResourceList.begin(); Iter != ResourceList.end(); Iter++)
    {
        TString Path = *Iter;
        TString RelPath = Path.ChopFront( ResDir.Size() );

        if (FileUtil::IsFile(Path) && Path.EndsWith(".rsmeta"))
        {
            // Determine resource name
            TString DirPath = RelPath.GetFileDirectory();
            TString CookedFilename = RelPath.GetFileName(false); // This call removes the .rsmeta extension
            TString ResName = CookedFilename.GetFileName(false); // This call removes the cooked extension
            ASSERT( IsValidResourcePath(DirPath, ResName) );

            // Determine resource type
            TString CookedExtension = CookedFilename.GetFileExtension();
            CResTypeInfo *pTypeInfo = CResTypeInfo::TypeForCookedExtension( Game(), CFourCC(CookedExtension) );

            if (!pTypeInfo)
            {
                errorf("Found resource but couldn't register because failed to identify resource type: %s", *RelPath);
                continue;
            }

            // Create resource entry
            CResourceEntry *pEntry = CResourceEntry::BuildFromDirectory(this, pTypeInfo, DirPath, ResName);

            // Validate the entry
            CAssetID ID = pEntry->ID();
            ASSERT( mResourceEntries.find(ID) == mResourceEntries.end() );
            ASSERT( ID.Length() == CAssetID::GameIDLength(mGame) );

            mResourceEntries[ID] = pEntry;
        }

        else if (FileUtil::IsDirectory(Path))
            CreateVirtualDirectory(RelPath);
    }

    // Generate new cache file
    if (ShouldGenerateCacheFile)
    {
        // Make sure gpResourceStore points to this store
        CResourceStore *pOldStore = gpResourceStore;
        gpResourceStore = this;

        // Make sure audio manager is loaded correctly so AGSC dependencies can be looked up
        if (mpProj)
            mpProj->AudioManager()->LoadAssets();

        // Update dependencies
        for (CResourceIterator It(this); It; ++It)
            It->UpdateDependencies();

        // Update database file
        mDatabaseCacheDirty = true;
        ConditionalSaveStore();

        // Restore old gpResourceStore
        gpResourceStore = pOldStore;
    }

    return true;
}

void CResourceStore::RebuildFromDirectory()
{
    if (mpProj)
        mpProj->AudioManager()->ClearAssets();

    ClearDatabase();
    BuildFromDirectory(true);
}

bool CResourceStore::IsResourceRegistered(const CAssetID& rkID) const
{
    return FindEntry(rkID) != nullptr;
}

CResourceEntry* CResourceStore::CreateNewResource(const CAssetID& rkID, EResourceType Type, const TString& rkDir, const TString& rkName, bool ExistingResource /*= false*/)
{
    CResourceEntry *pEntry = FindEntry(rkID);

    if (pEntry)
        errorf("Attempted to register resource that's already tracked in the database: %s / %s / %s", *rkID.ToString(), *rkDir, *rkName);

    else
    {
        // Validate directory
        if (IsValidResourcePath(rkDir, rkName))
        {
            pEntry = CResourceEntry::CreateNewResource(this, rkID, rkDir, rkName, Type, ExistingResource);
            mResourceEntries[rkID] = pEntry;
            mDatabaseCacheDirty = true;

            if (pEntry->IsLoaded())
            {
                TrackLoadedResource(pEntry);
            }

            debugf("CREATED NEW RESOURCE: [%s] %s", *rkID.ToString(), *pEntry->CookedAssetPath());
        }

        else
            errorf("Invalid resource path, failed to register: %s%s", *rkDir, *rkName);
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
        warnf("Can't find requested resource with ID \"%s\"", *rkID.ToString());
        return nullptr;
    }
}

CResource* CResourceStore::LoadResource(const CAssetID& rkID, EResourceType Type)
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

            errorf("Resource with ID \"%s\" requested with the wrong type; expected %s asset, get %s asset", *rkID.ToString(), *pExpectedType->TypeName(), *pGotType->TypeName());
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
    uint32 NumDeleted;

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
        errorf("Failed to open .contents.txt file: %s", *rkTxtPath);
        return;
    }

    while (!feof(pContentsFile))
    {
        // Get new line, parse to extract the ID/path
        char LineBuffer[512];
        fgets(LineBuffer, 512, pContentsFile);

        TString Line(LineBuffer);
        if (Line.IsEmpty()) break;

        uint32 IDStart = Line.IndexOfPhrase("0x") + 2;
        if (IDStart == 1) continue;

        uint32 IDEnd = Line.IndexOf(" \t", IDStart);
        uint32 PathStart = IDEnd + 1;
        uint32 PathEnd = Line.Size() - 5;

        TString IDStr = Line.SubString(IDStart, IDEnd - IDStart);
        TString Path = Line.SubString(PathStart, PathEnd - PathStart);

        CAssetID ID = CAssetID::FromString(IDStr);
        CResourceEntry *pEntry = FindEntry(ID);

        // Only process this entry if the ID exists
        if (pEntry)
        {
            // Chop name to just after "x_rep"
            uint32 RepStart = Path.IndexOfPhrase("_rep");

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

        pEntry->MoveAndRename(Dir, Name);
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

TString CResourceStore::StaticDefaultResourceDirPath(EGame Game)
{
    return (Game < EGame::CorruptionProto ? "Uncategorized/" : "uncategorized/");
}
