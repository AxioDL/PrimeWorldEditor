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
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Common/TString.h>
#include <iostream>

CResCache::CResCache()
    : mpGameExporter(nullptr)
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
    TString StringName = ResID.ToString() + "." + Type.ToString();

    // With Game Exporter - get data buffer from exporter
    if (mpGameExporter)
    {
        // Check if we already have resource loaded
        auto Got = mResourceCache.find(ResID.ToLongLong());
        if (Got != mResourceCache.end())
            return Got->second;

        // Otherwise load resource
        std::vector<u8> DataBuffer;
        mpGameExporter->LoadResource(ResID, DataBuffer);
        if (DataBuffer.empty()) return nullptr;

        CMemoryInStream MemStream(DataBuffer.data(), DataBuffer.size(), IOUtil::eBigEndian);
        CResource *pRes = InternalLoadResource(MemStream, ResID, Type);
        pRes->mResSource = StringName;
        return pRes;
    }

    // Without Game Exporter - load from file
    else
    {
        TString Source = mResDir + StringName;
        return GetResource(Source);
    }
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
    CFourCC Type = rkResPath.GetFileExtension().ToUpper();
    CResource *pRes = InternalLoadResource(File, ResID, Type);
    pRes->mResSource = rkResPath;

    // Add to cache and cleanup
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

// ************ PROTECTED ************
CResource* CResCache::InternalLoadResource(IInputStream& rInput, const CUniqueID& rkID, CFourCC Type)
{
    // todo - need some sort of auto-registration of loaders to avoid this if-else mess
    ASSERT(mResourceCache.find(rkID.ToLongLong()) == mResourceCache.end()); // this test should be done before calling this func!
    CResource *pRes = nullptr;

    // Load resource
    if      (Type == "CMDL") pRes = CModelLoader::LoadCMDL(rInput);
    else if (Type == "TXTR") pRes = CTextureDecoder::LoadTXTR(rInput);
    else if (Type == "ANCS") pRes = CAnimSetLoader::LoadANCS(rInput);
    else if (Type == "CHAR") pRes = CAnimSetLoader::LoadCHAR(rInput);
    else if (Type == "MREA") pRes = CAreaLoader::LoadMREA(rInput);
    else if (Type == "MLVL") pRes = CWorldLoader::LoadMLVL(rInput);
    else if (Type == "STRG") pRes = CStringLoader::LoadSTRG(rInput);
    else if (Type == "FONT") pRes = CFontLoader::LoadFONT(rInput);
    else if (Type == "SCAN") pRes = CScanLoader::LoadSCAN(rInput);
    else if (Type == "DCLN") pRes = CCollisionLoader::LoadDCLN(rInput);
    else if (Type == "EGMC") pRes = CPoiToWorldLoader::LoadEGMC(rInput);
    else if (Type == "CINF") pRes = CSkeletonLoader::LoadCINF(rInput);
    else if (Type == "ANIM") pRes = CAnimationLoader::LoadANIM(rInput);
    else if (Type == "CSKR") pRes = CSkinLoader::LoadCSKR(rInput);
    if (!pRes) pRes = new CResource(); // Default for unsupported formats

    ASSERT(pRes->mRefCount == 0);

    // Cache and return
    pRes->mID = rkID;
    mResourceCache[rkID.ToLongLong()] = pRes;
    return pRes;
}

CResCache gResCache;
