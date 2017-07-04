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

CResourceEntry::CResourceEntry(CResourceStore *pStore, const CAssetID& rkID,
               const TString& rkDir, const TString& rkFilename,
               EResType Type)
    : mpResource(nullptr)
    , mpStore(pStore)
    , mpDependencies(nullptr)
    , mID(rkID)
    , mpDirectory(nullptr)
    , mName(rkFilename)
    , mMetadataDirty(false)
    , mCachedSize(-1)
    , mCachedUppercaseName(rkFilename.ToUpper())
{
    mpTypeInfo = CResTypeInfo::FindTypeInfo(Type);
    ASSERT(mpTypeInfo);

    mpDirectory = mpStore->GetVirtualDirectory(rkDir, true);
    if (mpDirectory) mpDirectory->AddChild("", this);
}

CResourceEntry::~CResourceEntry()
{
    if (mpResource) delete mpResource;
    if (mpDependencies) delete mpDependencies;
}

bool CResourceEntry::LoadMetadata()
{
    ASSERT(!mMetadataDirty);
    TString Path = MetadataFilePath();

    if (FileUtil::Exists(Path))
    {
        // Validate file
        CFileInStream MetaFile(Path, IOUtil::eBigEndian);
        u32 Magic = MetaFile.ReadLong();

        if (Magic == FOURCC('META'))
        {
            CSerialVersion Version(MetaFile);
            CBinaryReader Reader(&MetaFile, Version);
            SerializeMetadata(Reader);
            return true;
        }
        else
        {
            Log::Error(Path + ": Failed to load metadata file, invalid magic: " + CFourCC(Magic).ToString());
        }
    }

    return false;
}

bool CResourceEntry::SaveMetadata(bool ForceSave /*= false*/)
{
    if (mMetadataDirty || ForceSave)
    {
        TString Path = MetadataFilePath();
        TString Dir = Path.GetFileDirectory();
        FileUtil::MakeDirectory(Dir);

        CFileOutStream MetaFile(Path, IOUtil::eBigEndian);

        if (MetaFile.IsValid())
        {
            MetaFile.WriteLong(0); // Magic dummy

            CSerialVersion Version(IArchive::skCurrentArchiveVersion, 0, Game());
            Version.Write(MetaFile);

            // Scope the binary writer to ensure it finishes before we go back to write the magic value
            {
                CBinaryWriter Writer(&MetaFile, Version);
                SerializeMetadata(Writer);
            }

            MetaFile.GoTo(0);
            MetaFile.WriteLong(FOURCC('META'));

            mMetadataDirty = false;
            return true;
        }
    }

    return false;
}

void CResourceEntry::SerializeMetadata(IArchive& rArc)
{
    // Serialize ID. If we already have a valid ID then don't allow the file to override it.
    CAssetID ID = mID;
    rArc << SERIAL("AssetID", ID);

    if (rArc.IsReader() && !mID.IsValid())
        mID = ID;

    // Serialize type
    rArc << SERIAL("Type", mpTypeInfo);

    // Serialize flags
    u32 Flags = mFlags & eREF_SavedFlags;
    rArc << SERIAL_AUTO(Flags);
    if (rArc.IsReader()) mFlags = Flags & eREF_SavedFlags;
}

void CResourceEntry::SerializeCacheData(IArchive& rArc)
{
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
        mpDependencies = new CDependencyTree();
        return;
    }

    bool WasLoaded = IsLoaded();

    if (!mpResource)
        Load();

    if (!mpResource)
    {
        Log::Error("Unable to update cached dependencies; failed to load resource");
        mpDependencies = new CDependencyTree();
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
    if (HasFlag(eREF_NeedsRecook)) return true;
    return (FileUtil::LastModifiedTime(CookedAssetPath()) < FileUtil::LastModifiedTime(RawAssetPath()));
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
        FileUtil::MakeDirectory(Dir);

        TString SerialName = mpTypeInfo->TypeName();
        SerialName.RemoveWhitespace();

        CXMLWriter Writer(Path, SerialName, 0, Game());
        mpResource->Serialize(Writer);

        if (!Writer.Save())
        {
            Log::Error("Failed to save raw resource: " + Path);
            DEBUG_BREAK;
            return false;
        }

        SetFlag(eREF_NeedsRecook);
    }

    // This resource type doesn't have a raw format; save cooked instead
    else
    {
        bool CookSuccess = Cook();

        if (!CookSuccess)
        {
            Log::Error("Failed to save resource: " + Name() + "." + CookedExtension().ToString());
            return false;
        }
    }

    // Resource has been saved; now make sure metadata, dependencies, and packages are all up to date
    SetFlag(eREF_HasBeenModified);
    SaveMetadata();
    UpdateDependencies();

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
    CFileOutStream File(Path, IOUtil::eBigEndian);
    if (!File.IsValid())
    {
        Log::Error("Failed to open cooked file for writing: " + Path);
        return false;
    }

    bool Success = CResourceCooker::CookResource(this, File);

    if (Success)
    {
        ClearFlag(eREF_NeedsRecook);
        SetFlag(eREF_HasBeenModified);
        SaveMetadata();
    }

    return Success;
}

CResource* CResourceEntry::Load()
{
    // If the asset is already loaded then just return it immediately
    if (mpResource) return mpResource;

    // Always try to load raw version as the raw version contains extra editor-only data.
    // If there is no raw version (which will be the case for resource types that don't
    // support serialization yet) then load the cooked version as a backup.
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
        CFileInStream File(CookedAssetPath(), IOUtil::eBigEndian);

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

bool CResourceEntry::CanMoveTo(const TString& rkDir, const TString& rkName)
{
    // Validate that the path/name are valid
    if (!mpStore->IsValidResourcePath(rkDir, rkName)) return false;

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

bool CResourceEntry::Move(const TString& rkDir, const TString& rkName, bool IsAutoGenDir /*= false*/, bool IsAutoGenName /*= false*/)
{
    if (!CanMoveTo(rkDir, rkName)) return false;

    // Store old paths
    CVirtualDirectory *pOldDir = mpDirectory;
    TString OldName = mName;
    TString OldCookedPath = CookedAssetPath();
    TString OldRawPath = RawAssetPath();
    TString OldMetaPath = MetadataFilePath();

    // Set new directory and name
    CVirtualDirectory *pNewDir = mpStore->GetVirtualDirectory(rkDir, true);
    if (pNewDir == mpDirectory && rkName == mName) return false;

    // Check if we can legally move to this spot
    ASSERT(pNewDir->FindChildResource(rkName, ResourceType()) == nullptr); // this check should be guaranteed to pass due to CanMoveTo() having already checked it

    mpDirectory = pNewDir;
    mName = rkName;
    TString NewCookedPath = CookedAssetPath();
    TString NewRawPath = RawAssetPath();
    TString NewMetaPath = MetadataFilePath();

    Log::Write("MOVING RESOURCE: " + FileUtil::MakeRelative(OldCookedPath, mpStore->ResourcesDir()) + " --> " + FileUtil::MakeRelative(NewCookedPath, mpStore->ResourcesDir()));

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
            FSMoveSuccess = FileUtil::CopyFile(OldRawPath, NewRawPath);

            if (!FSMoveSuccess)
                MoveFailReason = TString::Format("Failed to move raw file to new destination (%s --> %s)", *OldRawPath, *NewRawPath);
        }

        // Move cooked file to new location
        if (FSMoveSuccess && FileUtil::Exists(OldCookedPath))
        {
            FSMoveSuccess = FileUtil::CopyFile(OldCookedPath, NewCookedPath);

            if (!FSMoveSuccess)
            {
                FileUtil::DeleteFile(NewRawPath);
                MoveFailReason = TString::Format("Failed to move cooked file to new destination (%s --> %s)", *OldCookedPath, *NewCookedPath);
            }
        }

        // Move metadata file to new location
        if (FSMoveSuccess)
        {
            if (FileUtil::Exists(OldMetaPath))
            {
                FSMoveSuccess = FileUtil::CopyFile(OldMetaPath, NewMetaPath);

                if (!FSMoveSuccess)
                {
                    FileUtil::DeleteFile(NewRawPath);
                    FileUtil::DeleteFile(NewCookedPath);
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
        if (mpDirectory != pOldDir)
        {
            FSMoveSuccess = pOldDir->RemoveChildResource(this);
            ASSERT(FSMoveSuccess == true); // this shouldn't be able to fail
            mpDirectory->AddChild("", this);
            SetFlagEnabled(eREF_AutoResDir, IsAutoGenDir);
        }

        if (mName != OldName)
        {
            SetFlagEnabled(eREF_AutoResName, IsAutoGenName);
        }

        mpStore->SetDatabaseDirty();
        mCachedUppercaseName = rkName.ToUpper();
        FileUtil::DeleteFile(OldRawPath);
        FileUtil::DeleteFile(OldCookedPath);
        FileUtil::DeleteFile(OldMetaPath);

        SaveMetadata();
        return true;
    }

    // Otherwise, revert changes and let the caller know the move failed
    else
    {
        Log::Error("MOVE FAILED: " + MoveFailReason);
        mpDirectory = pOldDir;
        mName = OldName;
        mpStore->ConditionalDeleteDirectory(pNewDir, false);
        return false;
    }
}

CGameProject* CResourceEntry::Project() const
{
    return mpStore ? mpStore->Project() : nullptr;
}

EGame CResourceEntry::Game() const
{
    return mpStore ? mpStore->Game() : eUnknownGame;
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
