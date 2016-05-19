#include "CResCache.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Factory/CAnimationLoader.h"
#include "Core/Resource/Factory/CAnimSetLoader.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
#include "Core/Resource/Factory/CFontLoader.h"
#include "Core/Resource/Factory/CModelLoader.h"
#include "Core/Resource/Factory/CPoiToWorldLoader.h"
#include "Core/Resource/Factory/CScanLoader.h"
#include "Core/Resource/Factory/CSkeletonLoader.h"
#include "Core/Resource/Factory/CSkinLoader.h"
#include "Core/Resource/Factory/CStringLoader.h"
#include "Core/Resource/Factory/CTextureDecoder.h"
#include "Core/Resource/Factory/CWorldLoader.h"

#include <FileIO/FileIO.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/TString.h>
#include <iostream>

CResCache::CResCache()
{
}

CResCache::~CResCache()
{
    Clean();
}

void CResCache::Clean()
{
    if (mResourceCache.empty()) return;
    Log::Write("Cleaning unused resources");

    // I couldn't get this to work properly using reverse iterators, lol.
    // Resources get cached after their dependencies, which is why I go backwards
    // while loop is to ensure -all- unused resources are cleaned. Not sure of a better way to do it.
    int NumResourcesCleaned = 1;

    while (NumResourcesCleaned)
    {
        NumResourcesCleaned = 0;

        for (auto it = mResourceCache.end(); it != mResourceCache.begin();)
        {
            it--;
            if (it->second->mRefCount <= 0)
            {
                delete it->second;
                it = mResourceCache.erase(it);
                NumResourcesCleaned++;
            }
        }
    }
    Log::Write(std::to_string(mResourceCache.size()) + " resources loaded");
}

void CResCache::SetFolder(TString Path)
{
    Path.EnsureEndsWith("/");
    mResDir = Path;
    Log::Write("Set resource folder: " + Path);
}

TString CResCache::GetSourcePath()
{
    return mResDir;
}

CResource* CResCache::GetResource(CUniqueID ResID, CFourCC Type)
{
    if (!ResID.IsValid()) return nullptr;
    TString Source = mResDir + ResID.ToString() + "." + Type.ToString();
    return GetResource(Source);
}

CResource* CResCache::GetResource(const TString& rkResPath)
{
    CUniqueID ResID = rkResPath.Hash64();

    // Check if resource already exists
    auto Got = mResourceCache.find(ResID.ToLongLong());

    if (Got != mResourceCache.end())
        return Got->second;

    // Open file
    CFileInStream File(rkResPath.ToStdString(), IOUtil::eBigEndian);
    if (!File.IsValid())
    {
        Log::Error("Couldn't open resource: " + rkResPath);
        return nullptr;
    }

    // Save old ResDir to restore later
    TString OldResDir = mResDir;
    mResDir = rkResPath.GetFileDirectory();

    // Load resource
    CResource *pRes = nullptr;
    CFourCC Type = rkResPath.GetFileExtension().ToUpper();
    bool SupportedFormat = true;

    if      (Type == "CMDL") pRes = CModelLoader::LoadCMDL(File);
    else if (Type == "TXTR") pRes = CTextureDecoder::LoadTXTR(File);
    else if (Type == "ANCS") pRes = CAnimSetLoader::LoadANCS(File);
    else if (Type == "CHAR") pRes = CAnimSetLoader::LoadCHAR(File);
    else if (Type == "MREA") pRes = CAreaLoader::LoadMREA(File);
    else if (Type == "MLVL") pRes = CWorldLoader::LoadMLVL(File);
    else if (Type == "STRG") pRes = CStringLoader::LoadSTRG(File);
    else if (Type == "FONT") pRes = CFontLoader::LoadFONT(File);
    else if (Type == "SCAN") pRes = CScanLoader::LoadSCAN(File);
    else if (Type == "DCLN") pRes = CCollisionLoader::LoadDCLN(File);
    else if (Type == "EGMC") pRes = CPoiToWorldLoader::LoadEGMC(File);
    else if (Type == "CINF") pRes = CSkeletonLoader::LoadCINF(File);
    else if (Type == "ANIM") pRes = CAnimationLoader::LoadANIM(File);
    else if (Type == "CSKR") pRes = CSkinLoader::LoadCSKR(File);
    else SupportedFormat = false;

    if (!pRes) pRes = new CResource(); // Default for unsupported formats

    // Add to cache and cleanup
    pRes->mID = *rkResPath;
    pRes->mResSource = rkResPath;
    mResourceCache[ResID.ToLongLong()] = pRes;
    mResDir = OldResDir;
    return pRes;
}

CFourCC CResCache::FindResourceType(CUniqueID ResID, const TStringList& rkPossibleTypes)
{
    // If we only have one type then there's only one possibility.
    if (rkPossibleTypes.size() == 1)
        return CFourCC(rkPossibleTypes.front());

    // Determine extension from filesystem - try every extension until we find one that works
    TString PathBase = mResDir + ResID.ToString() + ".";

    for (auto it = rkPossibleTypes.begin(); it != rkPossibleTypes.end(); it++)
    {
        TString NewPath = PathBase + *it;

        if (FileUtil::Exists(NewPath))
            return CFourCC(*it);
    }

    return "UNKN";
}

void CResCache::CacheResource(CResource *pRes)
{
    u64 ID = pRes->ResID().ToLongLong();
    auto Got = mResourceCache.find(ID);

    if (Got != mResourceCache.end())
        mResourceCache[ID] = pRes;
}

void CResCache::DeleteResource(CUniqueID ResID)
{
    auto Got = mResourceCache.find(ResID.ToLongLong());

    if (Got != mResourceCache.end())
    {
        delete Got->second;
        mResourceCache.erase(Got, Got);
    }
}

CResCache gResCache;
