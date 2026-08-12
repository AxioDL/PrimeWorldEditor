#include "Core/GameProject/CResourceStore.h"

#include "Core/CAudioManager.h"
#include "Core/IUIRelay.h"
#include "Core/GameProject/CGameExporter.h"
#include "Core/GameProject/CGameProject.h"
#include "Core/Resource/CResource.h"
#include <Common/Macros.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/Serialization/CBasicBinaryReader.h>
#include <Common/Serialization/CBasicBinaryWriter.h>
#include <tinyxml2.h>

using namespace tinyxml2;
TString gDataDir;
bool gResourcesWritable = false;
bool gTemplatesWritable = false;
CResourceStore *gpResourceStore = nullptr;
CResourceStore *gpEditorStore = nullptr;

// Constructor for editor store
CResourceStore::CResourceStore(const TString& rkDatabasePath)
{
    mpDatabaseRoot = std::make_unique<CVirtualDirectory>(this);
    mDatabasePath = FileUtil::MakeAbsolute(rkDatabasePath.GetFileDirectory());
    if ((mDatabasePathExists = FileUtil::IsDirectory(mDatabasePath)))
        LoadDatabaseCache();
}

// Main constructor for game projects and game exporter
CResourceStore::CResourceStore(CGameProject *pProject)
    : mGame(EGame::Invalid)
{
    SetProject(pProject);
}

CResourceStore::~CResourceStore()
{
    CloseProject();
    DestroyUnreferencedResources();
}

static void RecursiveGetListOfEmptyDirectories(CVirtualDirectory *pDir, TStringList& rOutList)
{
    // Helper function for SerializeResourceDatabase
    if (pDir->IsEmpty(false))
    {
        rOutList.push_back(pDir->FullPath());
    }
    else
    {
        for (size_t SubIdx = 0; SubIdx < pDir->NumSubdirectories(); SubIdx++)
            RecursiveGetListOfEmptyDirectories(pDir->SubdirectoryByIndex(SubIdx), rOutList);
    }
}

bool CResourceStore::SerializeDatabaseCache(IArchive& rArc)
{
    // Serialize resources
    if (rArc.ParamBegin("Resources", 0))
    {
        // Serialize resources
        uint32_t ResourceCount = mResourceEntries.size();

        if (rArc.IsWriter())
        {
            // Make sure deleted resources aren't included in the count.
            // We can't use any of the resource view because they skip MarkedForDeletion resources.
            for (const auto& entry : mResourceEntries | std::views::values)
            {
                if (entry->IsMarkedForDeletion())
                    ResourceCount--;
            }
        }

        rArc << SerialParameter("ResourceCount", ResourceCount);

        if (rArc.IsReader())
        {
            for (uint32_t ResIdx = 0; ResIdx < ResourceCount; ResIdx++)
            {
                if (rArc.ParamBegin("Resource", 0))
                {
                    auto pEntry = CResourceEntry::BuildFromArchive(this, rArc);
                    ASSERT(FindEntry(pEntry->ID()) == nullptr);
                    mResourceEntries.insert_or_assign(pEntry->ID(), std::move(pEntry));
                    rArc.ParamEnd();
                }
            }
        }
        else
        {
            for (const auto& entry : MakeResourceView())
            {
                if (rArc.ParamBegin("Resource", 0))
                {
                    entry->SerializeEntryInfo(rArc, false);
                    rArc.ParamEnd();
                }
            }
        }
        rArc.ParamEnd();
    }

    // Serialize empty directory list
    TStringList EmptyDirectories;

    if (!rArc.IsReader())
        RecursiveGetListOfEmptyDirectories(mpDatabaseRoot.get(), EmptyDirectories);

    rArc << SerialParameter("EmptyDirectories", EmptyDirectories);

    if (rArc.IsReader())
    {
        for (const auto& dir : EmptyDirectories)
        {
            // Don't create empty virtual directories that don't actually exist in the filesystem
            const TString AbsPath = ResourcesDir() + dir;

            if (FileUtil::Exists(AbsPath))
                CreateVirtualDirectory(dir);
        }
    }

    return true;
}

bool CResourceStore::LoadDatabaseCache()
{
    ASSERT(!mDatabasePath.IsEmpty());
    const TString Path = DatabasePath();

    if (!mpDatabaseRoot)
        mpDatabaseRoot = std::make_unique<CVirtualDirectory>(this);

    // Load the resource database
    CBasicBinaryReader Reader(Path, FOURCC('CACH'));

    if (!Reader.IsValid() || !SerializeDatabaseCache(Reader))
    {
        if (GetUIRelay()->AskYesNoQuestion("Error", "Failed to load the resource database. Attempt to build from the directory? (This may take a while.)"))
        {
            if (!BuildFromDirectory(true))
                return false;
        }
        else
        {
            return false;
        }
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
    const TString Path = DatabasePath();
    NLog::Debug("Saving database cache...");

    CBasicBinaryWriter Writer(Path, FOURCC('CACH'), 0, mGame);

    if (!Writer.IsValid())
        return false;

    SerializeDatabaseCache(Writer);
    mDatabaseCacheDirty = false;
    return true;
}

void CResourceStore::ConditionalSaveStore()
{
    if (mDatabaseCacheDirty)
        SaveDatabaseCache();
}

void CResourceStore::SetProject(CGameProject *pProj)
{
    if (mpProj == pProj)
        return;

    if (mpProj)
        CloseProject();

    mpProj = pProj;

    if (!mpProj)
        return;

    mDatabasePath = mpProj->ProjectRoot();
    mpDatabaseRoot = std::make_unique<CVirtualDirectory>(this);
    mGame = mpProj->Game();

    // Clear deleted files from previous runs
    const TString DeletedPath = DeletedResourcePath();

    if (FileUtil::Exists(DeletedPath))
    {
        FileUtil::ClearDirectory(DeletedPath);
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
        NLog::Warn("{} resources still loaded on project close:", mLoadedResources.size());

        for (const auto& entry : mLoadedResources)
        {
            const CResourceEntry *pEntry = entry.second;
            NLog::Warn("\t{}.{}", pEntry->Name(), pEntry->CookedExtension().ToString());
        }

        ASSERT(false);
    }

    // Delete all entries from old project
    mResourceEntries.clear();

    // Clear deleted files from previous runs
    const TString DeletedPath = DeletedResourcePath();

    if (FileUtil::Exists(DeletedPath))
    {
        FileUtil::ClearDirectory(DeletedPath);
    }

    mpDatabaseRoot.reset();
    mpProj = nullptr;
    mGame = EGame::Invalid;
}

CVirtualDirectory* CResourceStore::GetVirtualDirectory(const TString& rkPath, bool AllowCreate)
{
    if (rkPath.IsEmpty())
        return mpDatabaseRoot.get();

    if (mpDatabaseRoot)
        return mpDatabaseRoot->FindChildDirectory(rkPath, AllowCreate);

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
    if (!rkID.IsValid())
        return nullptr;

    const auto Found = mResourceEntries.find(rkID);
    if (Found == mResourceEntries.cend())
        return nullptr;

    const auto& pEntry = Found->second;
    if (pEntry->IsMarkedForDeletion())
        return nullptr;

    return pEntry.get();
}

CResourceEntry* CResourceStore::FindEntry(const TString& rkPath) const
{
    return mpDatabaseRoot ? mpDatabaseRoot->FindChildResource(rkPath) : nullptr;
}

bool CResourceStore::AreAllEntriesValid() const
{
    return std::ranges::all_of(MakeResourceView(), [](const auto& entry) {
        return entry->HasCookedVersion() || entry->HasRawVersion();
    });
}

void CResourceStore::ClearDatabase()
{
    // THIS OPERATION REQUIRES THAT ALL RESOURCES ARE UNREFERENCED
    DestroyUnreferencedResources();

    if (!mLoadedResources.empty())
    {
        NLog::Debug("ERROR: Resources still loaded:");
        for (const auto& [asset, entry] : mLoadedResources)
            NLog::Debug("\t[{}] {}", asset.ToString(), entry->CookedAssetPath(true));
        ASSERT(false);
    }

    // Clear out existing resource entries and directories
    mResourceEntries.clear();

    mpDatabaseRoot = std::make_unique<CVirtualDirectory>(this);
    mDatabaseCacheDirty = true;
}

bool CResourceStore::BuildFromDirectory(bool ShouldGenerateCacheFile)
{
    ASSERT(mResourceEntries.empty());

    // Get list of resources
    const TString ResDir = ResourcesDir();
    TStringList ResourceList;
    FileUtil::GetDirectoryContents(ResDir, ResourceList);

    for (const auto& Path : ResourceList)
    {
        TString RelPath = Path.ChopFront(ResDir.Size());

        if (FileUtil::IsFile(Path) && Path.EndsWith(".rsmeta"))
        {
            // Determine resource name
            const TString DirPath = RelPath.GetFileDirectory();
            const TString CookedFilename = RelPath.GetFileName(false); // This call removes the .rsmeta extension
            const TString ResName = CookedFilename.GetFileName(false); // This call removes the cooked extension
            ASSERT(IsValidResourcePath(DirPath, ResName));

            // Determine resource type
            const TString CookedExtension = CookedFilename.GetFileExtension();
            CResTypeInfo* pTypeInfo = CResTypeInfo::TypeForCookedExtension(Game(), CFourCC(CookedExtension));

            if (!pTypeInfo)
            {
                NLog::Error("Found resource but couldn't register because failed to identify resource type: {}", RelPath);
                continue;
            }

            // Create resource entry
            auto pEntry = CResourceEntry::BuildFromDirectory(this, pTypeInfo, DirPath, ResName);

            // Validate the entry
            const CAssetID ID = pEntry->ID();
            ASSERT(!mResourceEntries.contains(ID));
            ASSERT(ID.Length() == CAssetID::GameIDLength(mGame));

            mResourceEntries.insert_or_assign(ID, std::move(pEntry));
        }
        else if (FileUtil::IsDirectory(Path))
        {
            CreateVirtualDirectory(RelPath);
        }
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
        for (auto& resource : MakeResourceView())
            resource->UpdateDependencies();

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
    {
        NLog::Error("Attempted to register resource that's already tracked in the database: {} / {} / {}", rkID.ToString(), rkDir, rkName);
    }
    else
    {
        // Validate directory
        if (IsValidResourcePath(rkDir, rkName))
        {
            auto res = CResourceEntry::CreateNewResource(this, rkID, rkDir, rkName, Type, ExistingResource);
            auto* resPtr = res.get();

            mResourceEntries.insert_or_assign(rkID, std::move(res));
            mDatabaseCacheDirty = true;

            if (resPtr->IsLoaded())
            {
                TrackLoadedResource(resPtr);
            }

            if (!ExistingResource)
            {
                NLog::Debug("CREATED NEW RESOURCE: [{}] {}", rkID.ToString(), resPtr->CookedAssetPath());
            }

            return resPtr;
        }

        NLog::Error("Invalid resource path, failed to register: {}{}", rkDir, rkName);
    }

    return pEntry;
}

CResource* CResourceStore::LoadResource(const CAssetID& rkID)
{
    if (!rkID.IsValid())
        return nullptr;

    CResourceEntry *pEntry = FindEntry(rkID);
    if (!pEntry)
    {
        // Resource doesn't seem to exist
        NLog::Warn("Can't find requested resource with ID \"{}\"", rkID.ToString());
        return nullptr;
    }

    return pEntry->Load();
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
            const CResTypeInfo* pExpectedType = CResTypeInfo::FindTypeInfo(Type);
            const CResTypeInfo* pGotType = pRes->TypeInfo();
            ASSERT(pExpectedType && pGotType);

            NLog::Error("Resource with ID \"{}\" requested with the wrong type; expected {} asset, got {} asset",
                        rkID.ToString(), pExpectedType->TypeName(), pGotType->TypeName());
            return nullptr;
        }
    }

    return nullptr;
}

CResource* CResourceStore::LoadResource(const TString& rkPath)
{
    // If this is a relative path then load via the resource DB
    CResourceEntry *pEntry = FindEntry(rkPath);

    if (pEntry)
    {
        // Verify extension matches the entry + load resource
        const TString Ext = rkPath.GetFileExtension();

        if (!Ext.IsEmpty())
        {
            if (Ext.Length() == 4)
            {
                ASSERT(Ext.CaseInsensitiveCompare(pEntry->CookedExtension().ToString()));
            }
            else
            {
                ASSERT(rkPath.EndsWith(pEntry->RawExtension()));
            }
        }

        return pEntry->Load();
    }

    return nullptr;
}

void CResourceStore::TrackLoadedResource(CResourceEntry *pEntry)
{
    ASSERT(pEntry->IsLoaded());
    ASSERT(!mLoadedResources.contains(pEntry->ID()));
    mLoadedResources.insert_or_assign(pEntry->ID(), pEntry);
}

void CResourceStore::DestroyUnreferencedResources()
{
    // This can be updated to avoid the do-while loop when reference lookup is implemented.
    uint32_t NumDeleted;

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
            else
            {
                ++It;
            }
        }
    } while (NumDeleted > 0);
}

bool CResourceStore::DeleteResourceEntry(CResourceEntry *pEntry)
{
    const CAssetID ID = pEntry->ID();

    if (pEntry->IsLoaded())
    {
        if (!pEntry->Unload())
            return false;

        const auto It = mLoadedResources.find(ID);
        ASSERT(It != mLoadedResources.end());
        mLoadedResources.erase(It);
    }

    if (pEntry->Directory())
        pEntry->Directory()->RemoveChildResource(pEntry);

    const auto It = mResourceEntries.find(ID);
    ASSERT(It != mResourceEntries.end());
    mResourceEntries.erase(It);

    delete pEntry;
    return true;
}

#ifdef _WIN32
static int wrap_fopen(FILE** pFile, const char *filename, const char *mode)
{
    return fopen_s(pFile, filename, mode);
}
#else
static int wrap_fopen(FILE** pFile, const char *filename, const char *mode)
{
  *pFile = fopen(filename, mode);
  return *pFile == nullptr;
}
#endif

void CResourceStore::ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly)
{
    // Read file contents -first- then move assets -after-; this
    // 1. avoids anything fucking up if the contents file is badly formatted and we crash, and
    // 2. avoids extra redundant moves (since there are redundant entries in the file)
    // todo: move to CAssetNameMap?
    std::map<CResourceEntry*, TString> PathMap;
    FILE *pContentsFile;
    wrap_fopen(&pContentsFile, rkTxtPath.CString(), "r");

    if (!pContentsFile)
    {
        NLog::Error("Failed to open .contents.txt file: {}", rkTxtPath);
        return;
    }

    while (!feof(pContentsFile))
    {
        // Get new line, parse to extract the ID/path
        char LineBuffer[512];
        std::fgets(LineBuffer, 512, pContentsFile);

        TString Line(LineBuffer);
        if (Line.IsEmpty())
            break;

        const uint64_t IDStart = Line.IndexOfPhrase("0x") + 2;
        if (IDStart == 1)
            continue;

        const uint64_t IDEnd = Line.IndexOf(" \t", IDStart);
        const uint64_t PathStart = IDEnd + 1;
        const uint64_t PathEnd = Line.Size() - 5;

        const TString IDStr = Line.SubString(IDStart, IDEnd - IDStart);
        TString Path = Line.SubString(PathStart, PathEnd - PathStart);

        const CAssetID ID = CAssetID::FromString(IDStr);
        CResourceEntry *pEntry = FindEntry(ID);

        // Only process this entry if the ID exists
        if (pEntry)
        {
            // Chop name to just after "x_rep"
            const auto RepStart = Path.IndexOfPhrase("_rep");

            if (RepStart != -1)
                Path = Path.ChopFront(RepStart + 5);

            // If the "x_rep" folder doesn't exist in this path for some reason, but this is still a path, then just chop off the drive letter.
            // Otherwise, this is most likely just a standalone name, so use the full name as-is.
            else if (Path[1] == ':')
                Path = Path.ChopFront(3);

            PathMap.insert_or_assign(pEntry, std::move(Path));
        }
    }

    std::fclose(pContentsFile);

    // Assign names
    for (auto& [entry, path] : PathMap)
    {
        if (UnnamedOnly && entry->IsNamed())
            continue;

        TString Dir = path.GetFileDirectory();
        const TString Name = path.GetFileName(false);
        if (Dir.IsEmpty())
            Dir = entry->DirectoryPath();

        entry->MoveAndRename(Dir, Name);
    }

    // Save
    ConditionalSaveStore();
}

bool CResourceStore::IsValidResourcePath(const TString& rkPath, const TString& rkName)
{
    // Path must not be an absolute path and must not go outside the project structure.
    // Name must not be a path.
    return CVirtualDirectory::IsValidDirectoryPath(rkPath) &&
           FileUtil::IsValidName(rkName, false) &&
           !rkName.Contains('/') &&
           !rkName.Contains('\\');
}

TString CResourceStore::StaticDefaultResourceDirPath(EGame Game)
{
    return Game < EGame::CorruptionProto ? "Uncategorized/" : "uncategorized/";
}
