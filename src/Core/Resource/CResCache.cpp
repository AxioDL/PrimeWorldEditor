#include "CResCache.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Factory/CAnimSetLoader.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
#include "Core/Resource/Factory/CFontLoader.h"
#include "Core/Resource/Factory/CModelLoader.h"
#include "Core/Resource/Factory/CPoiToWorldLoader.h"
#include "Core/Resource/Factory/CScanLoader.h"
#include "Core/Resource/Factory/CStringLoader.h"
#include "Core/Resource/Factory/CTextureDecoder.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include <Common/Log.h>

#include <Common/TString.h>
#include <FileIO/FileIO.h>
#include <iostream>
#include <boost/filesystem.hpp>

CResCache::CResCache()
    : mpPak(nullptr)
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
    mResSource.Path = Path;
    mResSource.Source = SResSource::eFolder;
    Log::Write("Set resource folder: " + Path);
}

void CResCache::SetPak(const TString& rkPath)
{
    CFileInStream *pPakFile = new CFileInStream(rkPath.ToStdString(), IOUtil::eBigEndian);

    if (!pPakFile->IsValid())
    {
        Log::Error("Couldn't load pak file: " + rkPath);
        delete pPakFile;
        return;
    }

    if (mpPak) delete mpPak;
    mpPak = new CPakFile(pPakFile);
    mResSource.Path = rkPath;
    mResSource.Source = SResSource::ePakFile;
    Log::Write("Loaded pak file: " + rkPath);
}

void CResCache::SetResSource(SResSource& rResSource)
{
    mResSource = rResSource;
}

SResSource CResCache::GetResSource()
{
    return mResSource;
}

TString CResCache::GetSourcePath()
{
    return mResSource.Path;
}

CResource* CResCache::GetResource(CUniqueID ResID, CFourCC Type)
{
    if (!ResID.IsValid()) return nullptr;

    auto got = mResourceCache.find(ResID.ToLongLong());

    if (got != mResourceCache.end())
        return got->second;

    std::vector<u8> *pBuffer = nullptr;
    TString Source;

    // Load from pak
    if (mResSource.Source == SResSource::ePakFile)
    {
        pBuffer = mpPak->Resource(ResID.ToLongLong(), Type);
        Source = ResID.ToString() + "." + Type.ToString();
    }

    // Load from folder
    else
    {
        Source = mResSource.Path + ResID.ToString() + "." + Type.ToString();
        CFileInStream File(Source.ToStdString(), IOUtil::eBigEndian);

        if (!File.IsValid())
        {
            Log::Error("Couldn't open resource: " + ResID.ToString() + "." + Type.ToString());
            return nullptr;
        }

        pBuffer = new std::vector<u8>;
        pBuffer->resize(File.Size());
        File.ReadBytes(pBuffer->data(), pBuffer->size());
    }
    if (!pBuffer) return nullptr;

    // Load resource
    CMemoryInStream Mem(pBuffer->data(), pBuffer->size(), IOUtil::eBigEndian);
    Mem.SetSourceString(*Source.GetFileName());
    CResource *pRes = nullptr;
    bool SupportedFormat = true;

    if      (Type == "CMDL") pRes = CModelLoader::LoadCMDL(Mem);
    else if (Type == "TXTR") pRes = CTextureDecoder::LoadTXTR(Mem);
    else if (Type == "ANCS") pRes = CAnimSetLoader::LoadANCS(Mem);
    else if (Type == "CHAR") pRes = CAnimSetLoader::LoadCHAR(Mem);
    else if (Type == "MREA") pRes = CAreaLoader::LoadMREA(Mem);
    else if (Type == "MLVL") pRes = CWorldLoader::LoadMLVL(Mem);
    else if (Type == "STRG") pRes = CStringLoader::LoadSTRG(Mem);
    else if (Type == "FONT") pRes = CFontLoader::LoadFONT(Mem);
    else if (Type == "SCAN") pRes = CScanLoader::LoadSCAN(Mem);
    else if (Type == "DCLN") pRes = CCollisionLoader::LoadDCLN(Mem);
    else if (Type == "EGMC") pRes = CPoiToWorldLoader::LoadEGMC(Mem);
    else SupportedFormat = false;

    // Log errors
    if (!SupportedFormat)
        Log::Write("Unsupported format; unable to load " + Type.ToString() + " " + ResID.ToString());

    if (!pRes) pRes = new CResource(); // Default for invalid resource or unsupported format

    // Add to cache and cleanup
    pRes->mID = ResID;
    pRes->mResSource = Source;
    mResourceCache[ResID.ToLongLong()] = pRes;
    delete pBuffer;
    return pRes;
}

CResource* CResCache::GetResource(const TString& rkResPath)
{
    // Since this function takes a string argument it always loads directly from a file - no pak
    CUniqueID ResID = rkResPath.Hash64();

    auto Got = mResourceCache.find(ResID.ToLongLong());

    if (Got != mResourceCache.end())
        return Got->second;

    CFileInStream File(rkResPath.ToStdString(), IOUtil::eBigEndian);
    if (!File.IsValid())
    {
        Log::Error("Couldn't open resource: " + rkResPath);
        return nullptr;
    }

    // Save old ResSource to restore later
    const SResSource OldSource = mResSource;
    mResSource.Source = SResSource::eFolder;
    mResSource.Path = rkResPath.GetFileDirectory();

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
    else SupportedFormat = false;

    if (!pRes) pRes = new CResource(); // Default for unsupported formats

    // Add to cache and cleanup
    pRes->mID = *rkResPath;
    pRes->mResSource = rkResPath;
    mResourceCache[ResID.ToLongLong()] = pRes;
    mResSource = OldSource;
    return pRes;
}

CFourCC CResCache::FindResourceType(CUniqueID ResID, const TStringList& rkPossibleTypes)
{
    // If we only have one type then there's only one possibility.
    if (rkPossibleTypes.size() == 1)
        return CFourCC(rkPossibleTypes.front());

    // Determine extension from pak
    if (mResSource.Source == SResSource::ePakFile)
    {
        for (auto it = rkPossibleTypes.begin(); it != rkPossibleTypes.end(); it++)
        {
            SResInfo ResInfo = mpPak->ResourceInfo(ResID.ToLongLong(), CFourCC(*it));

            if (ResInfo.Type != "NULL")
                return CFourCC(*it);
        }
    }

    // Determine extension from filesystem - try every extension until we find one that works
    else
    {
        TString PathBase = mResSource.Path + ResID.ToString() + ".";

        for (auto it = rkPossibleTypes.begin(); it != rkPossibleTypes.end(); it++)
        {
            TString NewPath = PathBase + *it;

            if (boost::filesystem::exists(NewPath.ToStdString()))
                return CFourCC(*it);
        }
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
