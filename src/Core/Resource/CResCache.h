#ifndef CRESCACHE_H
#define CRESCACHE_H

#include "CPakFile.h"
#include "CResource.h"
#include <Common/types.h>
#include <Common/TString.h>
#include <unordered_map>

struct SResSource
{
    TString Path;
    enum {
        Folder, PakFile
    } Source;
};

class CResCache
{
    std::unordered_map<u64, CResource*> mResourceCache;
    CPakFile *mpPak;
    SResSource mResSource;

public:
    CResCache();
    ~CResCache();
    void Clean();
    void SetFolder(TString path);
    void SetPak(const TString& path);
    void SetResSource(SResSource& ResSource);
    SResSource GetResSource();
    TString GetSourcePath();
    CResource* GetResource(CUniqueID ResID, CFourCC type);
    CResource* GetResource(const TString& ResPath);
    CFourCC FindResourceType(CUniqueID ResID, const TStringList& rkPossibleTypes);
    void CacheResource(CResource *pRes);
    void DeleteResource(CUniqueID ResID);
};

extern CResCache gResCache;

#endif // CRESCACHE_H
