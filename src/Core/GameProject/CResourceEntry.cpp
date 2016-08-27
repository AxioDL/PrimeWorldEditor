#include "CResourceEntry.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
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
    , mName(rkFilename)
    , mType(Type)
    , mCachedSize(-1)
    , mCachedUppercaseName(rkFilename.ToUpper())
{
    if (Transient) mFlags |= eREF_Transient;

    mpDirectory = mpStore->GetVirtualDirectory(rkDir, Transient, true);
    if (mpDirectory) mpDirectory->AddChild(L"", this);
    mGame = ((Transient || !mpStore->ActiveProject()) ? eUnknownGame : mpStore->ActiveProject()->Game());
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

    // todo: more robust method of skipping dependency updates. For now just skip the most
    // time-consuming type that can't have dependencies (textures).
    if (ResourceType() == eTexture)
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
    if (!WasLoaded)
        mpStore->DestroyUnreferencedResources();
}

TWideString CResourceEntry::CacheDataPath(bool Relative) const
{
    return mpStore->ActiveProject()->CacheDir(Relative) + mID.ToString().ToUTF16() + L".rcd";
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
    TWideString Ext = GetResourceRawExtension(mType, mGame).ToUTF16();
    TWideString Path = mpDirectory ? mpDirectory->FullPath() : L"";
    TWideString Name = mName + L"." + Ext;
    return ((IsTransient() || Relative) ? Path + Name : mpStore->ActiveProject()->ContentDir(false) + Path + Name);
}

TString CResourceEntry::CookedAssetPath(bool Relative) const
{
    TWideString Ext = GetResourceCookedExtension(mType, mGame).ToUTF16();
    TWideString Path = mpDirectory ? mpDirectory->FullPath() : L"";
    TWideString Name = mName + L"." + Ext;
    return ((IsTransient() || Relative) ? Path + Name : mpStore->ActiveProject()->CookedDir(false) + Path + Name);
}

CFourCC CResourceEntry::CookedExtension() const
{
    return CFourCC( GetResourceCookedExtension(mType, mGame) );
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
    if (ResourceSupportsSerialization(ResourceType()))
    {
        ShouldCollectGarbage = !IsLoaded();

        Load();
        if (!mpResource) return false;

        // Note: We call Serialize directly for resources to avoid having a redundant resource root node in the output file.
        TString Path = RawAssetPath();
        TString Dir = Path.GetFileDirectory();
        FileUtil::CreateDirectory(Dir.ToUTF16());

        CXMLWriter Writer(Path, GetResourceSerialName(ResourceType()), 0, mGame);
        mpResource->Serialize(Writer);
    }

    // Resource has been saved, now update dependencies + cache file
    UpdateDependencies();

    if (!SkipCacheSave)
        mpStore->SaveCacheFile();

    if (ShouldCollectGarbage)
        mpStore->DestroyUnreferencedResources();

    return true;
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
            CXMLReader Reader(RawAssetPath());
            mpResource->Serialize(Reader);
        }

        return mpResource;
    }

    else if (HasCookedVersion())
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

    mpResource = CResourceFactory::LoadCookedResource(this, rInput);
    mpStore->TrackLoadedResource(this);
    return mpResource;
}

bool CResourceEntry::Unload()
{
    ASSERT(mpResource != nullptr);
    delete mpResource;
    mpResource = nullptr;
    return true;
}

void CResourceEntry::Move(const TWideString& rkDir, const TWideString& rkName)
{
    // Store old paths
    TString OldCookedPath = CookedAssetPath();
    TString OldRawPath = RawAssetPath();

    // Set new directory and name
    bool HasDirectory = mpDirectory != nullptr;
    CVirtualDirectory *pNewDir = mpStore->GetVirtualDirectory(rkDir, IsTransient(), true);

    if (pNewDir != mpDirectory)
    {
        if (mpDirectory)
            mpDirectory->RemoveChildResource(this);
        mpDirectory = pNewDir;
    }

    if (mName != rkName)
        ASSERT(mpDirectory->FindChildResource(rkName) == nullptr);

    mName = rkName;
    mCachedUppercaseName = rkName.ToUpper();

    // Move files
    if (HasDirectory)
    {
        TString CookedPath = CookedAssetPath();
        TString RawPath = RawAssetPath();

        if (FileUtil::Exists(OldCookedPath) && CookedPath != OldCookedPath)
            FileUtil::MoveFile(OldCookedPath, CookedPath);

        if (FileUtil::Exists(OldRawPath) && RawPath != OldRawPath)
            FileUtil::MoveFile(OldRawPath, RawPath);
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
