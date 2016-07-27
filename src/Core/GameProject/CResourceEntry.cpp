#include "CResourceEntry.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include <FileIO/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>

// Resource Loaders
// todo: come up with a factory setup that doesn't suck
#include "Core/Resource/Factory/CAnimationLoader.h"
#include "Core/Resource/Factory/CAnimSetLoader.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
#include "Core/Resource/Factory/CDependencyGroupLoader.h"
#include "Core/Resource/Factory/CFontLoader.h"
#include "Core/Resource/Factory/CMaterialLoader.h"
#include "Core/Resource/Factory/CModelLoader.h"
#include "Core/Resource/Factory/CPoiToWorldLoader.h"
#include "Core/Resource/Factory/CScanLoader.h"
#include "Core/Resource/Factory/CScriptLoader.h"
#include "Core/Resource/Factory/CSkeletonLoader.h"
#include "Core/Resource/Factory/CSkinLoader.h"
#include "Core/Resource/Factory/CStringLoader.h"
#include "Core/Resource/Factory/CTextureDecoder.h"
#include "Core/Resource/Factory/CUnsupportedFormatLoader.h"
#include "Core/Resource/Factory/CUnsupportedParticleLoader.h"
#include "Core/Resource/Factory/CWorldLoader.h"

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
    mGame = ((Transient || !mpStore->ActiveProject()) ? eUnknownVersion : mpStore->ActiveProject()->Game());
}

CResourceEntry::~CResourceEntry()
{
    if (mpResource) delete mpResource;
}

bool CResourceEntry::LoadCacheData()
{
    ASSERT(!IsTransient());

    TWideString Path = CacheDataPath(false);
    CFileInStream File(Path.ToUTF8().ToStdString(), IOUtil::eLittleEndian);

    if (!File.IsValid())
    {
        Log::Error("Unable to load cache data " + Path.ToUTF8() + "; couldn't open file");
        return false;
    }

    // Header
    TString Magic = File.ReadString(4);
    ASSERT(Magic == "CACH");
    File.Seek(0x4, SEEK_CUR); // Skip Version
    mFlags = File.ReadLong() & eREF_SavedFlags;

    // Dependency Tree
    u32 DepsTreeSize = File.ReadLong();

    if (mpDependencies)
    {
        delete mpDependencies;
        mpDependencies = nullptr;
    }

    if (DepsTreeSize > 0)
    {
        mpDependencies = new CDependencyTree(mID);
        mpDependencies->Read(File, Game() <= eEchoes ? e32Bit : e64Bit);
    }

    return true;
}

bool CResourceEntry::SaveCacheData()
{
    ASSERT(!IsTransient());

    TWideString Path = CacheDataPath(false);
    TWideString Dir = Path.GetFileDirectory();
    FileUtil::CreateDirectory(Dir);
    CFileOutStream File(Path.ToUTF8().ToStdString(), IOUtil::eLittleEndian);

    if (!File.IsValid())
    {
        Log::Error("Unable to save cache data " + TString(Path.GetFileName()) + "; couldn't open file");
        return false;
    }

    // Header
    File.WriteString("CACH", 4);
    File.WriteLong(0); // Reserved Space (Version)
    File.WriteLong(mFlags & eREF_SavedFlags);

    // Dependency Tree
    if (!mpDependencies) UpdateDependencies();

    u32 DepsSizeOffset = File.Tell();
    File.WriteLong(0);

    u32 DepsStart = File.Tell();
    if (mpDependencies) mpDependencies->Write(File, Game() <= eEchoes ? e32Bit : e64Bit);
    u32 DepsSize = File.Tell() - DepsStart;
    File.Seek(DepsSizeOffset, SEEK_SET);
    File.WriteLong(DepsSize);

    // Thumbnail
    File.WriteLong(0); // Reserved Space (Thumbnail Size)

    return true;
}

void CResourceEntry::UpdateDependencies()
{
    if (mpDependencies)
    {
        delete mpDependencies;
        mpDependencies = nullptr;
    }

    if (!mpResource)
        Load();

    if (!mpResource)
    {
        Log::Error("Unable to update cached dependencies; failed to load resource");
        return;
    }

    mpDependencies = mpResource->BuildDependencyTree();
    gpResourceStore->DestroyUnreferencedResources();
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

CResource* CResourceEntry::Load()
{
    // todo: load raw
    if (mpResource) return mpResource;

    if (!HasCookedVersion())
    {
        Log::Error("Couldn't locate resource: " + CookedAssetPath(true));
        return nullptr;
    }

    CFileInStream File(CookedAssetPath().ToStdString(), IOUtil::eBigEndian);
    if (!File.IsValid())
    {
        Log::Error("Failed to open cooked resource: " + CookedAssetPath(true));
        return nullptr;
    }

    return Load(File);
}

CResource* CResourceEntry::Load(IInputStream& rInput)
{
    // Overload to allow for load from an arbitrary input stream.
    if (mpResource) return mpResource;
    if (!rInput.IsValid()) return nullptr;

    switch (mType)
    {
    case eAnimation:            mpResource = CAnimationLoader::LoadANIM(rInput, this);              break;
    case eAnimEventData:        mpResource = CUnsupportedFormatLoader::LoadEVNT(rInput, this);      break;
    case eAnimSet:              mpResource = CAnimSetLoader::LoadANCSOrCHAR(rInput, this);          break;
    case eArea:                 mpResource = CAreaLoader::LoadMREA(rInput, this);                   break;
    case eDependencyGroup:      mpResource = CDependencyGroupLoader::LoadDGRP(rInput, this);        break;
    case eDynamicCollision:     mpResource = CCollisionLoader::LoadDCLN(rInput, this);              break;
    case eFont:                 mpResource = CFontLoader::LoadFONT(rInput, this);                   break;
    case eHintSystem:           mpResource = CUnsupportedFormatLoader::LoadHINT(rInput, this);      break;
    case eMapWorld:             mpResource = CUnsupportedFormatLoader::LoadMAPW(rInput, this);      break;
    case eMapUniverse:          mpResource = CUnsupportedFormatLoader::LoadMAPU(rInput, this);      break;
    case eMidi:                 mpResource = CUnsupportedFormatLoader::LoadCSNG(rInput, this);      break;
    case eModel:                mpResource = CModelLoader::LoadCMDL(rInput, this);                  break;
    case eRuleSet:              mpResource = CUnsupportedFormatLoader::LoadRULE(rInput, this);      break;
    case eScan:                 mpResource = CScanLoader::LoadSCAN(rInput, this);                   break;
    case eSkeleton:             mpResource = CSkeletonLoader::LoadCINF(rInput, this);               break;
    case eSkin:                 mpResource = CSkinLoader::LoadCSKR(rInput, this);                   break;
    case eStaticGeometryMap:    mpResource = CPoiToWorldLoader::LoadEGMC(rInput, this);             break;
    case eStringTable:          mpResource = CStringLoader::LoadSTRG(rInput, this);                 break;
    case eTexture:              mpResource = CTextureDecoder::LoadTXTR(rInput, this);               break;
    case eWorld:                mpResource = CWorldLoader::LoadMLVL(rInput, this);                  break;

    case eParticle:
    case eParticleElectric:
    case eParticleSwoosh:
    case eParticleDecal:
    case eParticleWeapon:
    case eParticleCollisionResponse:
        mpResource = CUnsupportedParticleLoader::LoadParticle(rInput, this);
        break;

    default:                    mpResource = new CResource(this);                                   break;
    }

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
