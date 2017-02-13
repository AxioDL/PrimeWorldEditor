#include "CResourceEntry.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/Cooker/CResourceCooker.h"
#include "Core/Resource/Factory/CResourceFactory.h"
#include <FileIO/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/Serialization/CXMLReader.h>
#include <Common/Serialization/CXMLWriter.h>

CResourceEntry::CResourceEntry(CResourceStore *pStore, const CAssetID& rkID,
               const TWideString& rkDir, const TWideString& rkFilename,
               EResType Type, bool Transient /*= false*/)
    : mpResource(nullptr)
    , mpStore(pStore)
    , mpDependencies(nullptr)
    , mID(rkID)
    , mpDirectory(nullptr)
    , mName(rkFilename)
    , mCachedSize(-1)
    , mCachedUppercaseName(rkFilename.ToUpper())
{
    mpTypeInfo = CResTypeInfo::FindTypeInfo(Type);
    ASSERT(mpTypeInfo);

    if (Transient) mFlags |= eREF_Transient;

    mpDirectory = mpStore->GetVirtualDirectory(rkDir, Transient, true);
    if (mpDirectory) mpDirectory->AddChild(L"", this);
    mGame = ((Transient || !mpStore) ? eUnknownGame : mpStore->Game());
}

CResourceEntry::~CResourceEntry()
{
    if (mpResource) delete mpResource;
    if (mpDependencies) delete mpDependencies;
}

void CResourceEntry::SerializeCacheData(IArchive& rArc)
{
    ASSERT(!IsTransient());

    u32 Flags = mFlags & eREF_SavedFlags;
    rArc << SERIAL_AUTO(Flags);
    if (rArc.IsReader()) mFlags = Flags & eREF_SavedFlags;

    // Note: If the dependency tree format is changed this should be adjusted so that
    // we regenerate the dependencies from scratch instead of reading the tree if the
    // file version number is too low
    rArc << SERIAL_ABSTRACT("Dependencies", mpDependencies, &gDependencyNodeFactory);
}

void CResourceEntry::UpdateDependencies()
{
    if (mpDependencies)
    {
        delete mpDependencies;
        mpDependencies = nullptr;
    }

    if (!mpTypeInfo->CanHaveDependencies())
    {
        mpDependencies = new CDependencyTree(ID());
        return;
    }

    bool WasLoaded = IsLoaded();

    if (!mpResource)
        Load();

    if (!mpResource)
    {
        Log::Error("Unable to update cached dependencies; failed to load resource");
        return;
    }

    mpDependencies = mpResource->BuildDependencyTree();
    mpStore->SetCacheDataDirty();

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
    TWideString Ext = RawExtension().ToUTF16();
    TWideString Path = mpDirectory ? mpDirectory->FullPath() : L"";
    TWideString Name = mName + L"." + Ext;
    return ((IsTransient() || Relative) ? Path + Name : mpStore->RawDir(false) + Path + Name);
}

TString CResourceEntry::RawExtension() const
{
    return mpTypeInfo->RawExtension();
}

TString CResourceEntry::CookedAssetPath(bool Relative) const
{
    TWideString Ext = CookedExtension().ToString().ToUTF16();
    TWideString Path = mpDirectory ? mpDirectory->FullPath() : L"";
    TWideString Name = mName + L"." + Ext;
    return ((IsTransient() || Relative) ? Path + Name : mpStore->CookedDir(false) + Path + Name);
}

CFourCC CResourceEntry::CookedExtension() const
{
    return mpTypeInfo->CookedExtension(mGame);
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

u64 CResourceEntry::Size() const
{
    if (mCachedSize == -1)
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
    if (mFlags.HasFlag(eREF_NeedsRecook)) return true;
    return (FileUtil::LastModifiedTime(CookedAssetPath()) < FileUtil::LastModifiedTime(RawAssetPath()));
}

void CResourceEntry::SetGame(EGame NewGame)
{
    if (mGame != NewGame)
    {
        // todo: implement checks here. This needs work because we should trigger a recook and if the extension changes
        // we should delete the old file. Also a lot of resources can't evaluate this correctly due to file version
        // numbers being shared between games.
        mGame = NewGame;
    }
}

bool CResourceEntry::Save(bool SkipCacheSave /*= false*/)
{
    // SkipCacheSave argument tells us not to save the resource cache file. This is generally not advised because we don't
    // want the actual resource data to desync from the cache data. However, there are occasions where we save multiple
    // resources at a time and in that case it's preferable to only save the cache once. If you do set SkipCacheSave to true,
    // then make sure you manually update the cache file afterwards.
    //
    // For now, always save the resource when this function is called even if there's been no changes made to it in memory.
    // In the future this might not be desired behavior 100% of the time.
    // We also might want this function to trigger a cook for certain resource types eventually.
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
        FileUtil::MakeDirectory(Dir.ToUTF16());

        TString SerialName = mpTypeInfo->TypeName();
        SerialName.RemoveWhitespace();

        CXMLWriter Writer(Path, SerialName, 0, mGame);
        mpResource->Serialize(Writer);

        if (!Writer.Save())
        {
            Log::Error("Failed to save raw resource: " + Path);
            DEBUG_BREAK;
            return false;
        }

        mFlags |= eREF_NeedsRecook;
    }

    // This resource type doesn't have a raw format; save cooked instead
    else
    {
        bool CookSuccess = Cook();

        if (!CookSuccess)
        {
            Log::Error("Failed to save resource: " + Name().ToUTF8() + "." + CookedExtension().ToString());
            return false;
        }
    }

    // Resource has been saved; now make sure dependencies, cache data, and packages are all up to date
    mFlags |= eREF_HasBeenModified;

    UpdateDependencies();
    mpStore->SetCacheDataDirty();

    if (!SkipCacheSave)
    {
        mpStore->ConditionalSaveStore();

        // Flag dirty any packages that contain this resource.
        for (u32 iPkg = 0; iPkg < mpStore->Project()->NumPackages(); iPkg++)
        {
            CPackage *pPkg = mpStore->Project()->PackageByIndex(iPkg);

            if (pPkg->ContainsAsset(ID()))
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
    CFileOutStream File(Path.ToStdString(), IOUtil::eBigEndian);
    if (!File.IsValid())
    {
        Log::Error("Failed to open cooked file for writing: " + Path);
        return false;
    }

    bool Success = CResourceCooker::CookResource(this, File);

    if (Success)
    {
        mFlags &= ~eREF_NeedsRecook;
        mFlags |= eREF_HasBeenModified;
        mpStore->SetCacheDataDirty();
    }

    return Success;
}

CResource* CResourceEntry::Load()
{
    // Always try to load raw version as the raw version contains extra editor-only data.
    // If there is no raw version (which will be the case for resource types that don't
    // support serialization yet) then load the cooked version as a backup.
    if (mpResource) return mpResource;

    if (HasRawVersion())
    {
        mpResource = CResourceFactory::SpawnResource(this);

        if (mpResource)
        {
            // Set gpResourceStore to ensure the correct resource store is accessed by loader functions
            CResourceStore *pOldStore = gpResourceStore;
            gpResourceStore = mpStore;

            CXMLReader Reader(RawAssetPath());

            if (!Reader.IsValid())
            {
                Log::Error("Failed to load raw resource; falling back on cooked. Raw path: " + RawAssetPath());
                delete mpResource;
                mpResource = nullptr;
            }

            else
            {
                mpResource->Serialize(Reader);
                mpStore->TrackLoadedResource(this);
                gpResourceStore = pOldStore;
            }
        }

        if (mpResource)
            return mpResource;
    }

    ASSERT(!mpResource);
    if (HasCookedVersion())
    {
        CFileInStream File(CookedAssetPath().ToStdString(), IOUtil::eBigEndian);

        if (!File.IsValid())
        {
            Log::Error("Failed to open cooked resource: " + CookedAssetPath(true));
            return nullptr;
        }

        return LoadCooked(File);
    }

    else
    {
        Log::Error("Couldn't locate resource: " + CookedAssetPath(true));
        return nullptr;
    }
}

CResource* CResourceEntry::LoadCooked(IInputStream& rInput)
{
    // Overload to allow for load from an arbitrary input stream.
    if (mpResource) return mpResource;
    if (!rInput.IsValid()) return nullptr;

    // Set gpResourceStore to ensure the correct resource store is accessed by loader functions
    CResourceStore *pOldStore = gpResourceStore;
    gpResourceStore = mpStore;

    mpResource = CResourceFactory::LoadCookedResource(this, rInput);
    if (mpResource)
        mpStore->TrackLoadedResource(this);

    gpResourceStore = pOldStore;
    return mpResource;
}

bool CResourceEntry::Unload()
{
    ASSERT(mpResource != nullptr);
    ASSERT(!mpResource->IsReferenced());
    delete mpResource;
    mpResource = nullptr;
    return true;
}

bool CResourceEntry::CanMoveTo(const TWideString& rkDir, const TWideString& rkName)
{
    // Transient resources can't be moved
    if (IsTransient()) return false;

    // Validate that the path/name are valid file paths
    if (!FileUtil::IsValidPath(rkDir, true) || !FileUtil::IsValidName(rkName, false)) return false;

    // We need to validate the path isn't taken already - either the directory doesn't exist, or doesn't have a resource by this name
    CVirtualDirectory *pDir = mpStore->GetVirtualDirectory(rkDir, false, false);
    if (pDir && pDir->FindChildResource(rkName, ResourceType())) return false;

    // All checks are true
    return true;
}

bool CResourceEntry::Move(const TWideString& rkDir, const TWideString& rkName)
{
    if (!CanMoveTo(rkDir, rkName)) return false;

    // Store old paths
    CVirtualDirectory *pOldDir = mpDirectory;
    TWideString OldName = mName;
    TString OldCookedPath = CookedAssetPath();
    TString OldRawPath = RawAssetPath();

    // Set new directory and name
    CVirtualDirectory *pNewDir = mpStore->GetVirtualDirectory(rkDir, IsTransient(), true);
    if (pNewDir == mpDirectory && rkName == mName) return false;

    // Check if we can legally move to this spot
    ASSERT(pNewDir->FindChildResource(rkName, ResourceType()) == nullptr); // this check should be guaranteed to pass due to CanMoveTo() having already checked it

    mpDirectory = pNewDir;
    mName = rkName;
    TString NewCookedPath = CookedAssetPath();
    TString NewRawPath = RawAssetPath();

    Log::Write("MOVING RESOURCE: " + FileUtil::MakeRelative(OldCookedPath, mpStore->CookedDir(false)).ToUTF8() + " --> " + FileUtil::MakeRelative(NewCookedPath, mpStore->CookedDir(false)).ToUTF8());

    // If the old/new paths are the same then we should have already exited as CanMoveTo() should have returned false
    ASSERT(OldCookedPath != NewCookedPath && OldRawPath != NewRawPath);

    // The cooked/raw asset paths should not exist right now!!!
    bool FSMoveSuccess = false;
    TString MoveFailReason;

    if (!HasRawVersion() && !HasCookedVersion())
    {
        FSMoveSuccess = true;

        if (FileUtil::Exists(OldRawPath))
        {
            FSMoveSuccess = FileUtil::CopyFile(OldRawPath, NewRawPath);

            if (!FSMoveSuccess)
            {
                FileUtil::DeleteFile(NewRawPath);
                MoveFailReason = TString::Format("Failed to move raw file to new destination (%s --> %s)", *OldRawPath, *NewRawPath);
            }
        }

        if (FSMoveSuccess && FileUtil::Exists(OldCookedPath))
        {
            FSMoveSuccess = FileUtil::CopyFile(OldCookedPath, NewCookedPath);

            if (!FSMoveSuccess)
            {
                FileUtil::DeleteFile(NewCookedPath);
                MoveFailReason = TString::Format("Failed to move cooked file to new destination (%s --> %s)", *OldCookedPath, *NewCookedPath);
            }
        }
    }
    else
    {
        bool HasRaw = HasRawVersion();
        MoveFailReason = TString::Format("File already exists at %s asset destination (%s)", HasRaw ? "raw" : "cooked", HasRaw ? *NewRawPath : *NewCookedPath);
    }

    // If we succeeded, finish the move
    if (FSMoveSuccess)
    {
        if (mpDirectory != pOldDir)
        {
            FSMoveSuccess = pOldDir->RemoveChildResource(this);
            ASSERT(FSMoveSuccess == true); // this shouldn't be able to fail
            mpDirectory->AddChild(L"", this);
            mpStore->ConditionalDeleteDirectory(pOldDir);
        }

        mpStore->SetDatabaseDirty();
        mCachedUppercaseName = rkName.ToUpper();
        FileUtil::DeleteFile(OldRawPath);
        FileUtil::DeleteFile(OldCookedPath);
        return true;
    }

    // Otherwise, revert changes and let the caller know the move failed
    else
    {
        Log::Error("MOVE FAILED: " + MoveFailReason);
        mpDirectory = pOldDir;
        mName = OldName;
        mpStore->ConditionalDeleteDirectory(pNewDir);
        return false;
    }
}

void CResourceEntry::AddToProject(const TWideString& rkDir, const TWideString& rkName)
{
    if (mFlags.HasFlag(eREF_Transient))
    {
        mFlags.ClearFlag(eREF_Transient);
        Move(rkDir, rkName);
    }

    else
    {
        Log::Error("AddToProject called on non-transient resource entry: " + CookedAssetPath(true));
    }
}

void CResourceEntry::RemoveFromProject()
{
    if (!mFlags.HasFlag(eREF_Transient))
    {
        TString Dir = CookedAssetPath().GetFileDirectory();
        mFlags.SetFlag(eREF_Transient);
        Move(Dir, mName);
    }

    else
    {
        Log::Error("RemoveFromProject called on transient resource entry: " + CookedAssetPath(true));
    }
}

CGameProject* CResourceEntry::Project() const
{
    return mpStore->Project();
}
