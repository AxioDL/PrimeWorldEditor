#ifndef CRESCACHE_H
#define CRESCACHE_H

#include "CResource.h"
#include <Common/types.h>
#include <Common/TString.h>
#include <unordered_map>

class CResCache
{
    std::unordered_map<u64, CResource*> mResourceCache;
    TString mResDir;

public:
    CResCache();
    ~CResCache();
    void Clean();
    void SetFolder(TString Path);
    TString GetSourcePath();
    CResource* GetResource(CUniqueID ResID, CFourCC Type);
    CResource* GetResource(const TString& rkResPath);
    CFourCC FindResourceType(CUniqueID ResID, const TStringList& rkPossibleTypes);
    void CacheResource(CResource *pRes);
    void DeleteResource(CUniqueID ResID);
};

extern CResCache gResCache;

#endif // CRESCACHE_H
