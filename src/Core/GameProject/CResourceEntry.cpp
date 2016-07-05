#include "CResourceEntry.h"
#include "CGameProject.h"
#include "CResourceStore.h"
#include "Core/Resource/CResource.h"
#include <Common/FileUtil.h>
#include <Common/TString.h>

// Resource Loaders
// todo: come up with a factory setup that doesn't suck
#include "Core/Resource/Factory/CAnimationLoader.h"
#include "Core/Resource/Factory/CAnimSetLoader.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
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
#include "Core/Resource/Factory/CWorldLoader.h"

CResourceEntry::CResourceEntry(CResourceStore *pStore, const CUniqueID& rkID,
               const TWideString& rkDir, const TWideString& rkFilename,
               EResType Type, bool Transient /*= false*/)
    : mpStore(pStore)
    , mpResource(nullptr)
    , mID(rkID)
    , mName(rkFilename)
    , mType(Type)
    , mNeedsRecook(false)
    , mTransient(Transient)
{
    mpDirectory = mpStore->GetVirtualDirectory(rkDir, Transient, true);
    if (mpDirectory) mpDirectory->AddChild(L"", this);
    mGame = ((mTransient || !mpStore->ActiveProject()) ? eUnknownVersion : mpStore->ActiveProject()->Game());
}

CResourceEntry::~CResourceEntry()
{
    if (mpResource) delete mpResource;
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
    return ((mTransient || Relative) ? Path + Name : mpStore->ActiveProject()->ContentDir(false) + Path + Name);
}

TString CResourceEntry::CookedAssetPath(bool Relative) const
{
    TWideString Ext = GetResourceCookedExtension(mType, mGame).ToUTF16();
    TWideString Path = mpDirectory ? mpDirectory->FullPath() : L"";
    TWideString Name = mName + L"." + Ext;
    return ((mTransient || Relative) ? Path + Name : mpStore->ActiveProject()->CookedDir(false) + Path + Name);
}

bool CResourceEntry::NeedsRecook() const
{
    // Assets that do not have a raw version can't be recooked since they will always just be saved cooked to begin with.
    // We will recook any asset where the raw version has been updated but not recooked yet. mNeedsRecook can also be
    // toggled to arbitrarily flag any asset for recook.
    if (!HasRawVersion()) return false;
    if (!HasCookedVersion()) return true;
    if (mNeedsRecook) return true;
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
    case eAnimation:            mpResource = CAnimationLoader::LoadANIM(rInput, this);      break;
    case eAnimSet:              mpResource = CAnimSetLoader::LoadANCSOrCHAR(rInput, this);  break;
    case eArea:                 mpResource = CAreaLoader::LoadMREA(rInput, this);           break;
    case eDynamicCollision:     mpResource = CCollisionLoader::LoadDCLN(rInput, this);      break;
    case eFont:                 mpResource = CFontLoader::LoadFONT(rInput, this);           break;
    case eModel:                mpResource = CModelLoader::LoadCMDL(rInput, this);          break;
    case eScan:                 mpResource = CScanLoader::LoadSCAN(rInput, this);           break;
    case eSkeleton:             mpResource = CSkeletonLoader::LoadCINF(rInput, this);       break;
    case eSkin:                 mpResource = CSkinLoader::LoadCSKR(rInput, this);           break;
    case eStaticGeometryMap:    mpResource = CPoiToWorldLoader::LoadEGMC(rInput, this);     break;
    case eStringTable:          mpResource = CStringLoader::LoadSTRG(rInput, this);         break;
    case eTexture:              mpResource = CTextureDecoder::LoadTXTR(rInput, this);       break;
    case eWorld:                mpResource = CWorldLoader::LoadMLVL(rInput, this);          break;
    default:                    mpResource = new CResource(this);                           break;
    }

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
    CVirtualDirectory *pNewDir = mpStore->GetVirtualDirectory(rkDir, mTransient, true);

    if (pNewDir != mpDirectory)
    {
        if (mpDirectory)
            mpDirectory->RemoveChildResource(this);
        mpDirectory = pNewDir;
    }

    if (mName != rkName)
        ASSERT(mpDirectory->FindChildResource(rkName) == nullptr);

    mName = rkName;

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
    if (mTransient)
    {
        mTransient = false;
        Move(rkDir, rkName);
    }

    else
    {
        Log::Error("AddToProject called on non-transient resource entry: " + CookedAssetPath(true));
    }
}

void CResourceEntry::RemoveFromProject()
{
    if (!mTransient)
    {
        TString Dir = CookedAssetPath().GetFileDirectory();
        mTransient = true;
        Move(Dir, mName);
    }

    else
    {
        Log::Error("RemoveFromProject called on transient resource entry: " + CookedAssetPath(true));
    }
}
