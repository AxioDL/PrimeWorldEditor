#include "CResourceEntry.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/Cooker/CResourceCooker.h"
#include "Core/Resource/Factory/CResourceFactory.h"
#include <Common/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/Serialization/CXMLReader.h>
#include <Common/Serialization/CXMLWriter.h>

CResourceEntry::CResourceEntry(CResourceStore *pStore)
    : mpStore(pStore)
    , mID(CAssetID::InvalidID(pStore->Game()))
{}

// Static constructors
std::unique_ptr<CResourceEntry> CResourceEntry::CreateNewResource(CResourceStore *pStore, const CAssetID& rkID,
                                                                  const TString& rkDir, const TString& rkName,
                                                                  EResourceType Type, bool ExistingResource)
{
    // Initialize all entry info with the input data.
    auto pEntry = std::unique_ptr<CResourceEntry>(new CResourceEntry(pStore));
    pEntry->mID = rkID;
    pEntry->mName = rkName;
    pEntry->mCachedUppercaseName = rkName.ToUpper();

    pEntry->mpTypeInfo = CResTypeInfo::FindTypeInfo(Type);
    ASSERT(pEntry->mpTypeInfo);

    pEntry->mpDirectory = pStore->GetVirtualDirectory(rkDir, true);
    ASSERT(pEntry->mpDirectory);
    pEntry->mpDirectory->AddChild("", pEntry.get());

    pEntry->mMetadataDirty = true;

    // If this is a new resource (i.e. not a base game resource that we are currently exporting),
    // then instantiate the new resource data so it can be saved as soon as possible.
    if (!ExistingResource)
    {
        pEntry->mpResource = CResourceFactory::CreateResource(pEntry.get());

        if (pEntry->mpResource)
        {
            pEntry->mpResource->InitializeNewResource();
        }
    }

    return pEntry;
}

std::unique_ptr<CResourceEntry> CResourceEntry::BuildFromArchive(CResourceStore *pStore, IArchive& rArc)
{
    // Load all entry info from the archive.
    auto pEntry = std::unique_ptr<CResourceEntry>(new CResourceEntry(pStore));
    pEntry->SerializeEntryInfo(rArc, false);
    ASSERT(pEntry->mpTypeInfo);
    ASSERT(pEntry->mpDirectory);
    return pEntry;
}

std::unique_ptr<CResourceEntry> CResourceEntry::BuildFromDirectory(CResourceStore *pStore, CResTypeInfo *pTypeInfo,
                                                                   const TString& rkDirPath, const TString& rkName)
{
    // Initialize as much entry info as possible from the input data, then load the rest from the metadata file.
    ASSERT(pTypeInfo);

    auto pEntry = std::unique_ptr<CResourceEntry>(new CResourceEntry(pStore));
    pEntry->mpTypeInfo = pTypeInfo;
    pEntry->mName = rkName;
    pEntry->mCachedUppercaseName = rkName.ToUpper();

    pEntry->mpDirectory = pStore->GetVirtualDirectory(rkDirPath, true);
    ASSERT(pEntry->mpDirectory);
    pEntry->mpDirectory->AddChild("", pEntry.get());

    // Make sure we're valid, then load the remaining data from the metadata file
    ASSERT(pEntry->HasCookedVersion() || pEntry->HasRawVersion());
    bool Success = pEntry->LoadMetadata();
    ASSERT(Success);

    return pEntry;
}

CResourceEntry::~CResourceEntry() = default;

bool CResourceEntry::LoadMetadata()
{
    ASSERT(!mMetadataDirty);

    TString Path = MetadataFilePath();
    CBinaryReader MetaFile(Path, FOURCC('META'));

    if (MetaFile.IsValid())
    {
        SerializeEntryInfo(MetaFile, true);
        return true;
    }
    else
    {
        errorf("%s: Failed to load metadata file!", *Path);
    }

    return false;
}

bool CResourceEntry::SaveMetadata(bool ForceSave /*= false*/)
{
    // Make sure we aren't saving a deleted resource
    ASSERT( !HasFlag(EResEntryFlag::MarkedForDeletion) );

    if (mMetadataDirty || ForceSave)
    {
        TString Path = MetadataFilePath();
        TString Dir = Path.GetFileDirectory();
        FileUtil::MakeDirectory(Dir);

        CBinaryWriter MetaFile(Path, FOURCC('META'), 0, Game());

        if (MetaFile.IsValid())
        {
            SerializeEntryInfo(MetaFile, true);
            mMetadataDirty = false;
            return true;
        }
    }

    return false;
}

void CResourceEntry::SerializeEntryInfo(IArchive& rArc, bool MetadataOnly)
{
    CAssetID ID = mID;

    rArc << SerialParameter("AssetID", ID)
         << SerialParameter("Type", mpTypeInfo)
         << SerialParameter("Flags", mFlags);

    // Don't allow the file to override our asset ID if we already have a valid one.
    if (rArc.IsReader() && !mID.IsValid())
        mID = ID;

    // Serialize extra data that we exclude from the metadata file
    if (!MetadataOnly)
    {
        TString Dir = (mpDirectory ? mpDirectory->FullPath() : "");

        rArc << SerialParameter("Name", mName)
             << SerialParameter("Directory", Dir)
             << SerialParameter("Dependencies", mpDependencies);

        if (rArc.IsReader())
        {
            mpDirectory = mpStore->GetVirtualDirectory(Dir, true);
            mpDirectory->AddChild("", this);
            mCachedUppercaseName = mName.ToUpper();
        }
    }
}

void CResourceEntry::UpdateDependencies()
{
    mpDependencies.reset();

    if (!mpTypeInfo->CanHaveDependencies())
    {
        mpDependencies = std::make_unique<CDependencyTree>();
        return;
    }

    bool WasLoaded = IsLoaded();

    if (!mpResource)
        Load();

    if (!mpResource)
    {
        errorf("Unable to update cached dependencies; failed to load resource");
        mpDependencies = std::make_unique<CDependencyTree>();
        return;
    }

    mpDependencies = mpResource->BuildDependencyTree();
    mpStore->SetCacheDirty();

    if (!WasLoaded)
        mpStore->DestroyUnreferencedResources();
}

bool CResourceEntry::HasRawVersion() const
{
    return FileUtil::Exists(RawAssetPath());
}

bool CResourceEntry::HasCookedVersion() const
{
    return FileUtil::Exists(CookedAssetPath());
}

TString CResourceEntry::RawAssetPath(bool Relative) const
{
    return CookedAssetPath(Relative) + ".rsraw";
}

TString CResourceEntry::RawExtension() const
{
    return CookedExtension().ToString() + ".rsraw";
}

TString CResourceEntry::CookedAssetPath(bool Relative) const
{
    TString Ext = CookedExtension().ToString();
    TString Path = mpDirectory ? mpDirectory->FullPath() : "";
    TString Name = mName + "." + Ext;
    return Relative ? Path + Name : mpStore->ResourcesDir() + Path + Name;
}

CFourCC CResourceEntry::CookedExtension() const
{
    return mpTypeInfo->CookedExtension(Game());
}

TString CResourceEntry::MetadataFilePath(bool Relative) const
{
    return CookedAssetPath(Relative) + ".rsmeta";
}

bool CResourceEntry::IsInDirectory(CVirtualDirectory *pDir) const
{
    CVirtualDirectory *pParentDir = mpDirectory;

    while (pParentDir)
    {
        if (pParentDir == pDir) return true;
        pParentDir = pParentDir->Parent();
    }

    return false;
}

uint64 CResourceEntry::Size() const
{
    if (mCachedSize == UINT64_MAX)
    {
        if (HasCookedVersion())
            mCachedSize = FileUtil::FileSize(CookedAssetPath());
        else
            return 0;
    }

    return mCachedSize;
}

bool CResourceEntry::NeedsRecook() const
{
    // Assets that do not have a raw version can't be recooked since they will always just be saved cooked to begin with.
    // We will recook any asset where the raw version has been updated but not recooked yet. eREF_NeedsRecook can also be
    // toggled to arbitrarily flag any asset for recook.
    if (!HasRawVersion()) return false;
    if (!HasCookedVersion()) return true;
    if (HasFlag(EResEntryFlag::NeedsRecook)) return true;
    return (FileUtil::LastModifiedTime(CookedAssetPath()) < FileUtil::LastModifiedTime(RawAssetPath()));
}

bool CResourceEntry::Save(bool SkipCacheSave /*= false*/, bool FlagForRecook /*= true*/)
{
    // SkipCacheSave argument tells us not to save the resource cache file. This is generally not advised because we don't
    // want the actual resource data to desync from the cache data. However, there are occasions where we save multiple
    // resources at a time and in that case it's preferable to only save the cache once. If you do set SkipCacheSave to true,
    // then make sure you manually update the cache file afterwards.
    //
    // For now, always save the resource when this function is called even if there's been no changes made to it in memory.
    // In the future this might not be desired behavior 100% of the time.
    bool ShouldCollectGarbage = false;

    // Save raw resource
    if (mpTypeInfo->CanBeSerialized())
    {
        ShouldCollectGarbage = !IsLoaded();

        Load();
        if (!mpResource) return false;

        // Note: We call Serialize directly for resources to avoid having a redundant resource root node in the output file.
        TString Path = RawAssetPath();
        TString Dir = Path.GetFileDirectory();
        FileUtil::MakeDirectory(Dir);

        TString SerialName = mpTypeInfo->TypeName();
        SerialName.RemoveWhitespace();

        CXMLWriter Writer(Path, SerialName, 0, Game());
        mpResource->Serialize(Writer);

        if (!Writer.Save())
        {
            errorf("Failed to save raw resource: %s", *Path);
            return false;
        }

        if (FlagForRecook)
        {
            SetFlag(EResEntryFlag::NeedsRecook);
        }
    }

    // This resource type doesn't have a raw format; save cooked instead
    else
    {
        bool CookSuccess = Cook();

        if (!CookSuccess)
        {
            errorf("Failed to save resource: %s.%s", *Name(), *CookedExtension().ToString());
            return false;
        }
    }

    // Resource has been saved; now make sure metadata, dependencies, and packages are all up to date
    SetFlag(EResEntryFlag::HasBeenModified);
    SaveMetadata();
    UpdateDependencies();

    if (!SkipCacheSave)
    {
        mpStore->ConditionalSaveStore();
    }

    // Flag dirty any packages that contain this resource.
    if (FlagForRecook)
    {
        for (size_t iPkg = 0; iPkg < mpStore->Project()->NumPackages(); iPkg++)
        {
            CPackage *pPkg = mpStore->Project()->PackageByIndex(iPkg);

            if (!pPkg->NeedsRecook() && pPkg->ContainsAsset(ID()))
                pPkg->MarkDirty();
        }
    }

    if (ShouldCollectGarbage)
        mpStore->DestroyUnreferencedResources();

    return true;
}

bool CResourceEntry::Cook()
{
    Load();
    if (!mpResource) return false;

    TString Path = CookedAssetPath();
    TString Dir = Path.GetFileDirectory();
    FileUtil::MakeDirectory(Dir);

    // Attempt to open output cooked file
    CFileOutStream File(Path, EEndian::BigEndian);
    if (!File.IsValid())
    {
        errorf("Failed to open cooked file for writing: %s", *Path);
        return false;
    }

    bool Success = CResourceCooker::CookResource(this, File);

    if (Success)
    {
        ClearFlag(EResEntryFlag::NeedsRecook);
        SetFlag(EResEntryFlag::HasBeenModified);
        SaveMetadata();
    }

    return Success;
}

CResource* CResourceEntry::Load()
{
    // If the asset is already loaded then just return it immediately
    if (mpResource)
        return mpResource.get();

    // Always try to load raw version as the raw version contains extra editor-only data.
    // If there is no raw version (which will be the case for resource types that don't
    // support serialization yet) then load the cooked version as a backup.
    if (HasRawVersion())
    {
        mpResource = CResourceFactory::CreateResource(this);

        if (mpResource)
        {
            // Set gpResourceStore to ensure the correct resource store is accessed by loader functions
            CResourceStore *pOldStore = gpResourceStore;
            gpResourceStore = mpStore;

            CXMLReader Reader(RawAssetPath());

            if (!Reader.IsValid())
            {
                errorf("Failed to load raw resource; falling back on cooked. Raw path: %s", *RawAssetPath());
                mpResource.reset();
            }

            else
            {
                mpResource->Serialize(Reader);
                mpStore->TrackLoadedResource(this);
                gpResourceStore = pOldStore;
            }
        }

        if (mpResource)
            return mpResource.get();
    }

    ASSERT(!mpResource);
    if (HasCookedVersion())
    {
        CFileInStream File(CookedAssetPath(), EEndian::BigEndian);

        if (!File.IsValid())
        {
            errorf("Failed to open cooked resource: %s", *CookedAssetPath(true));
            return nullptr;
        }

        return LoadCooked(File);
    }

    else
    {
        errorf("Couldn't locate resource: %s", *CookedAssetPath(true));
        return nullptr;
    }
}

CResource* CResourceEntry::LoadCooked(IInputStream& rInput)
{
    // Overload to allow for load from an arbitrary input stream.
    if (mpResource)
        return mpResource.get();

    if (!rInput.IsValid())
        return nullptr;

    // Set gpResourceStore to ensure the correct resource store is accessed by loader functions
    CResourceStore *pOldStore = gpResourceStore;
    gpResourceStore = mpStore;

    mpResource = CResourceFactory::LoadCookedResource(this, rInput);
    if (mpResource)
        mpStore->TrackLoadedResource(this);

    gpResourceStore = pOldStore;
    return mpResource.get();
}

bool CResourceEntry::Unload()
{
    ASSERT(mpResource != nullptr);
    ASSERT(!mpResource->IsReferenced());
    mpResource.reset();
    return true;
}

bool CResourceEntry::CanMoveTo(const TString& rkDir, const TString& rkName)
{
    // Validate that the path/name are valid
    if (!CResourceStore::IsValidResourcePath(rkDir, rkName))
        return false;

    // We need to validate the path isn't taken already - either the directory doesn't exist, or doesn't have a resource by this name
    CVirtualDirectory *pDir = mpStore->GetVirtualDirectory(rkDir, false);

    if (pDir)
    {
        if (pDir == mpDirectory && rkName == mName)
            return false;

        if (pDir->FindChildResource(rkName, ResourceType()))
            return false;
    }

    // All checks are true
    return true;
}

bool CResourceEntry::MoveAndRename(const TString& rkDir, const TString& rkName, bool IsAutoGenDir /*= false*/, bool IsAutoGenName /*= false*/)
{
    // Make sure we are not moving a deleted resource.
    ASSERT( !IsMarkedForDeletion() );

    if (!CanMoveTo(rkDir, rkName)) return false;

    // Store old paths
    CVirtualDirectory *pOldDir = mpDirectory;
    TString OldName = mName;
    TString OldCookedPath = CookedAssetPath();
    TString OldRawPath = RawAssetPath();
    TString OldMetaPath = MetadataFilePath();

    // Set new directory and name
    bool DirAlreadyExisted = true;
    CVirtualDirectory *pNewDir = mpStore->GetVirtualDirectory(rkDir, false);

    if (!pNewDir)
    {
        pNewDir = mpStore->GetVirtualDirectory(rkDir, true);
        DirAlreadyExisted = false;
    }

    if (pNewDir == mpDirectory && rkName == mName) return false;

    // Check if we can legally move to this spot
    ASSERT(pNewDir->FindChildResource(rkName, ResourceType()) == nullptr); // this check should be guaranteed to pass due to CanMoveTo() having already checked it

    mpDirectory = pNewDir;
    mName = rkName;
    TString NewCookedPath = CookedAssetPath();
    TString NewRawPath = RawAssetPath();
    TString NewMetaPath = MetadataFilePath();

    debugf("MOVING RESOURCE: %s --> %s",
           *FileUtil::MakeRelative(OldCookedPath, mpStore->ResourcesDir()),
           *FileUtil::MakeRelative(NewCookedPath, mpStore->ResourcesDir())
    );

    // If the old/new paths are the same then we should have already exited as CanMoveTo() should have returned false
    ASSERT(OldCookedPath != NewCookedPath && OldRawPath != NewRawPath && OldMetaPath != NewMetaPath);

    // The cooked/raw asset paths should not exist right now!!!
    bool FSMoveSuccess = false;
    TString MoveFailReason;

    if (!FileUtil::Exists(NewCookedPath) && !FileUtil::Exists(NewRawPath) && !FileUtil::Exists(NewMetaPath))
    {
        FSMoveSuccess = true;

        // Move raw file to new location
        if (FileUtil::Exists(OldRawPath))
        {
            FSMoveSuccess = FileUtil::MoveFile(OldRawPath, NewRawPath);

            if (!FSMoveSuccess)
                MoveFailReason = TString::Format("Failed to move raw file to new destination (%s --> %s)", *OldRawPath, *NewRawPath);
        }

        // Move cooked file to new location
        if (FSMoveSuccess && FileUtil::Exists(OldCookedPath))
        {
            FSMoveSuccess = FileUtil::MoveFile(OldCookedPath, NewCookedPath);

            if (!FSMoveSuccess)
            {
                MoveFailReason = TString::Format("Failed to move cooked file to new destination (%s --> %s)", *OldCookedPath, *NewCookedPath);
            }
        }

        // Move metadata file to new location
        if (FSMoveSuccess)
        {
            if (FileUtil::Exists(OldMetaPath))
            {
                FSMoveSuccess = FileUtil::MoveFile(OldMetaPath, NewMetaPath);

                if (!FSMoveSuccess)
                {
                    MoveFailReason = TString::Format("Failed to move metadata file to new destination (%s --> %s)", *OldMetaPath, *NewMetaPath);
                }
            }

            else
                mMetadataDirty = true;
        }
    }
    else
    {
        bool HasCooked = FileUtil::Exists(NewCookedPath);
        bool HasRaw = FileUtil::Exists(NewRawPath);

        TString BadFileType = HasCooked ? "cooked"      : (HasRaw ? "raw"      : "metadata");
        TString BadFilePath = HasCooked ? NewCookedPath : (HasRaw ? NewRawPath : NewMetaPath);

        MoveFailReason = TString::Format("File already exists at %s asset destination (%s)", *BadFileType, *BadFilePath);
    }

    // If we succeeded, finish the move
    if (FSMoveSuccess)
    {
        if (mpDirectory != pOldDir && pOldDir != nullptr)
        {
            FSMoveSuccess = pOldDir->RemoveChildResource(this);
            ASSERT(FSMoveSuccess == true); // this shouldn't be able to fail
            mpDirectory->AddChild("", this);
            SetFlagEnabled(EResEntryFlag::AutoResDir, IsAutoGenDir);
        }

        if (mName != OldName)
        {
            SetFlagEnabled(EResEntryFlag::AutoResName, IsAutoGenName);
        }

        mpStore->SetCacheDirty();
        mCachedUppercaseName = rkName.ToUpper();
        SaveMetadata();
        return true;
    }

    // Otherwise, revert changes and let the caller know the move failed
    else
    {
        errorf("MOVE FAILED: %s", *MoveFailReason);
        mpDirectory = pOldDir;
        mName = OldName;

        if (!DirAlreadyExisted)
        {
            mpStore->ConditionalDeleteDirectory(pNewDir, true);
        }

        if (FileUtil::Exists(NewRawPath))
            FileUtil::MoveFile(NewRawPath, OldRawPath);

        if (FileUtil::Exists(NewCookedPath))
            FileUtil::MoveFile(NewCookedPath, OldCookedPath);

        return false;
    }
}

bool CResourceEntry::Move(const TString& rkDir, bool IsAutoGenDir /*= false*/)
{
    return MoveAndRename(rkDir, mName, IsAutoGenDir, false);
}

bool CResourceEntry::Rename(const TString& rkName, bool IsAutoGenName /*= false*/)
{
    return MoveAndRename(mpDirectory->FullPath(), rkName, false, IsAutoGenName);
}

void CResourceEntry::MarkDeleted(bool InDeleted)
{
    // Flags resource for future deletion. "Deleted" resources remain in memory (which
    // allows them to easily be un-deleted) but cannot be looked up in the resource
    // store and will not save back out to the resource database. Their file data is
    // stored in a temporary directory, which allows them to be moved back if the user
    // un-does the deletion.
    if (IsMarkedForDeletion() != InDeleted)
    {
        SetFlagEnabled(EResEntryFlag::MarkedForDeletion, InDeleted);

        // Restore old name/directory if un-deleting
        if (!InDeleted)
        {
            // Our directory path is stored in the Name field - see below for explanation
            int NameEnd = mName.IndexOf('|');
            ASSERT( NameEnd != -1 );

            TString DirPath = mName.ChopFront(NameEnd + 1);
            mName = mName.ChopBack( mName.Size() - NameEnd);
            mpDirectory = mpStore->GetVirtualDirectory( DirPath, true );
            ASSERT( mpDirectory != nullptr );
            mpDirectory->AddChild("", this);
        }

        TString CookedPath = CookedAssetPath();
        TString RawPath = RawAssetPath();
        TString MetaPath = MetadataFilePath();

        TString PathBase = mpStore->DeletedResourcePath() + mID.ToString() + ".";
        TString DelCookedPath = PathBase + CookedExtension().ToString();
        TString DelRawPath = DelCookedPath + ".rsraw";
        TString DelMetaPath = DelCookedPath + ".rsmeta";

        // If we are deleting...
        if (InDeleted)
        {
            // Temporarily store our directory path in the name string.
            // This is a hack, but we can't store the directory pointer because it may have been
            // deleted and remade by the user by the time the resource is un-deleted, which
            // means it is not safe to access later. Separating the name and the path with
            // the '|' character is safe because this character is not allowed in filenames
            // (which is enforced in FileUtil::IsValidName()).
            mName = mName + "|" + mpDirectory->FullPath();

            // Remove from parent directory.
            mpDirectory->RemoveChildResource(this);
            mpDirectory = nullptr;

            // Move any resource files out of the project into a temporary folder.
            FileUtil::MakeDirectory(DelMetaPath.GetFileDirectory());

            if (FileUtil::Exists(MetaPath))
            {
                FileUtil::MoveFile(MetaPath, DelMetaPath);
            }
            if (FileUtil::Exists(RawPath))
            {
                FileUtil::MoveFile(RawPath, DelRawPath);
            }
            if (FileUtil::Exists(CookedPath))
            {
                FileUtil::MoveFile(CookedPath, DelCookedPath);
            }
        }
        // If we are un-deleting...
        else
        {
            // Move any resource files out of the temporary folder back into the project.
            FileUtil::MakeDirectory(MetaPath.GetFileDirectory());

            if (FileUtil::Exists(DelMetaPath))
            {
                FileUtil::MoveFile(DelMetaPath, MetaPath);
            }
            if (FileUtil::Exists(DelRawPath))
            {
                FileUtil::MoveFile(DelRawPath, RawPath);
            }
            if (FileUtil::Exists(DelCookedPath))
            {
                FileUtil::MoveFile(DelCookedPath, CookedPath);
            }
        }

        mpStore->SetCacheDirty();
        debugf("%s FOR DELETION: [%s] %s", InDeleted ? "MARKED" : "UNMARKED", *ID().ToString(), *CookedPath.GetFileName());
    }
}

CGameProject* CResourceEntry::Project() const
{
    return mpStore ? mpStore->Project() : nullptr;
}

EGame CResourceEntry::Game() const
{
    return mpStore ? mpStore->Game() : EGame::Invalid;
}

void CResourceEntry::SetFlag(EResEntryFlag Flag)
{
    if (!HasFlag(Flag))
    {
        mFlags.SetFlag(Flag);
        mMetadataDirty = true;
    }
}

void CResourceEntry::ClearFlag(EResEntryFlag Flag)
{
    if (HasFlag(Flag))
    {
        mFlags.ClearFlag(Flag);
        mMetadataDirty = true;
    }
}
