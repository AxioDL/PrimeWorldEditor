#include "CResCache.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Factory/CAnimSetLoader.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
#include "Core/Resource/Factory/CFontLoader.h"
#include "Core/Resource/Factory/CModelLoader.h"
#include "Core/Resource/Factory/CScanLoader.h"
#include "Core/Resource/Factory/CStringLoader.h"
#include "Core/Resource/Factory/CTextureDecoder.h"
#include "Core/Resource/Factory/CWorldLoader.h"
#include "Core/Log.h"

#include <Common/TString.h>
#include <FileIO/FileIO.h>
#include <iostream>

CResCache::CResCache()
{
    mpPak = nullptr;
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
    int numResourcesCleaned = 1;

    while (numResourcesCleaned)
    {
        numResourcesCleaned = 0;

        for (auto it = mResourceCache.end(); it != mResourceCache.begin();)
        {
            it--;
            if (it->second->mRefCount <= 0)
            {
                delete it->second;
                it = mResourceCache.erase(it);
                numResourcesCleaned++;
            }
        }
    }
    Log::Write(std::to_string(mResourceCache.size()) + " resources loaded");
}

void CResCache::SetFolder(TString path)
{
    path.EnsureEndsWith("/");
    mResSource.Path = path;
    mResSource.Source = SResSource::Folder;
    Log::Write("Set resource folder: " + path);
}

void CResCache::SetPak(const TString& path)
{
    CFileInStream *pakfile = new CFileInStream(path.ToStdString(), IOUtil::BigEndian);
    if (!pakfile->IsValid())
    {
        Log::Error("Couldn't load pak file: " + path);
        delete pakfile;
        return;
    }

    if (mpPak) delete mpPak;
    mpPak = new CPakFile(pakfile);
    mResSource.Path = path;
    mResSource.Source = SResSource::PakFile;
    Log::Write("Loaded pak file: " + path);
}

void CResCache::SetResSource(SResSource& ResSource)
{
    mResSource = ResSource;
}

SResSource CResCache::GetResSource()
{
    return mResSource;
}

TString CResCache::GetSourcePath()
{
    return mResSource.Path;
}

CResource* CResCache::GetResource(CUniqueID ResID, CFourCC type)
{
    if (!ResID.IsValid()) return nullptr;

    auto got = mResourceCache.find(ResID.ToLongLong());

    if (got != mResourceCache.end())
        return got->second;

    std::vector<u8> *pBuffer = nullptr;
    TString Source;

    // Load from pak
    if (mResSource.Source == SResSource::PakFile)
    {
        pBuffer = mpPak->getResource(ResID.ToLongLong(), type);
        Source = ResID.ToString() + "." + type.ToString();
    }

    // Load from folder
    else
    {
        Source = mResSource.Path + ResID.ToString() + "." + type.ToString();
        CFileInStream file(Source.ToStdString(), IOUtil::BigEndian);
        if (!file.IsValid())
        {
            Log::Error("Couldn't open resource: " + ResID.ToString() + "." + type.ToString());
            return nullptr;
        }

        pBuffer = new std::vector<u8>;
        pBuffer->resize(file.Size());
        file.ReadBytes(pBuffer->data(), pBuffer->size());
    }
    if (!pBuffer) return nullptr;

    // Load resource
    CMemoryInStream mem(pBuffer->data(), pBuffer->size(), IOUtil::BigEndian);
    mem.SetSourceString(*Source.GetFileName());
    CResource *Res = nullptr;
    bool SupportedFormat = true;

    if      (type == "CMDL") Res = CModelLoader::LoadCMDL(mem);
    else if (type == "TXTR") Res = CTextureDecoder::LoadTXTR(mem);
    else if (type == "ANCS") Res = CAnimSetLoader::LoadANCS(mem);
    else if (type == "CHAR") Res = CAnimSetLoader::LoadCHAR(mem);
    else if (type == "MREA") Res = CAreaLoader::LoadMREA(mem);
    else if (type == "MLVL") Res = CWorldLoader::LoadMLVL(mem);
    else if (type == "STRG") Res = CStringLoader::LoadSTRG(mem);
    else if (type == "FONT") Res = CFontLoader::LoadFONT(mem);
    else if (type == "SCAN") Res = CScanLoader::LoadSCAN(mem);
    else if (type == "DCLN") Res = CCollisionLoader::LoadDCLN(mem);
    else SupportedFormat = false;

    // Log errors
    if (!SupportedFormat)
        Log::Write("Unsupported format; unable to load " + type.ToString() + " " + ResID.ToString());

    if (!Res) Res = new CResource(); // Default for invalid resource or unsupported format

    // Add to cache and cleanup
    Res->mID = ResID;
    Res->mResSource = Source;
    mResourceCache[ResID.ToLongLong()] = Res;
    delete pBuffer;
    return Res;
}

CResource* CResCache::GetResource(const TString& ResPath)
{
    // Since this function takes a string argument it always loads directly from a file - no pak
    CUniqueID ResID = ResPath.Hash64();

    auto got = mResourceCache.find(ResID.ToLongLong());

    if (got != mResourceCache.end())
        return got->second;

    CFileInStream file(ResPath.ToStdString(), IOUtil::BigEndian);
    if (!file.IsValid())
    {
        Log::Error("Couldn't open resource: " + ResPath);
        return nullptr;
    }

    // Save old ResSource to restore later
    const SResSource OldSource = mResSource;
    mResSource.Source = SResSource::Folder;
    mResSource.Path = ResPath.GetFileDirectory();

    // Load resource
    CResource *Res = nullptr;
    CFourCC type = ResPath.GetFileExtension().ToUpper();
    bool SupportedFormat = true;

    if      (type == "CMDL") Res = CModelLoader::LoadCMDL(file);
    else if (type == "TXTR") Res = CTextureDecoder::LoadTXTR(file);
    else if (type == "ANCS") Res = CAnimSetLoader::LoadANCS(file);
    else if (type == "CHAR") Res = CAnimSetLoader::LoadCHAR(file);
    else if (type == "MREA") Res = CAreaLoader::LoadMREA(file);
    else if (type == "MLVL") Res = CWorldLoader::LoadMLVL(file);
    else if (type == "FONT") Res = CFontLoader::LoadFONT(file);
    else if (type == "SCAN") Res = CScanLoader::LoadSCAN(file);
    else if (type == "DCLN") Res = CCollisionLoader::LoadDCLN(file);
    else SupportedFormat = false;

    if (!Res) Res = new CResource(); // Default for unsupported formats

    // Add to cache and cleanup
    Res->mID = *ResPath;
    Res->mResSource = ResPath;
    mResourceCache[ResID.ToLongLong()] = Res;
    mResSource = OldSource;
    return Res;
}

void CResCache::CacheResource(CResource *pRes)
{
    u64 ID = pRes->ResID().ToLongLong();
    auto got = mResourceCache.find(ID);

    if (got != mResourceCache.end())
        mResourceCache[ID] = pRes;
}

void CResCache::DeleteResource(CUniqueID ResID)
{
    auto got = mResourceCache.find(ResID.ToLongLong());

    if (got != mResourceCache.end())
    {
        delete got->second;
        mResourceCache.erase(got, got);
    }
}

CResCache gResCache;
