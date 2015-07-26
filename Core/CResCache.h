#ifndef CRESCACHE_H
#define CRESCACHE_H

#include <Common/types.h>
#include <Resource/CPakFile.h>
#include <Resource/CResource.h>
#include <unordered_map>

struct SResSource
{
    std::string Path;
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
    void SetFolder(std::string path);
    void SetPak(std::string path);
    void SetResSource(SResSource& ResSource);
    SResSource GetResSource();
    std::string GetSourcePath();
    CResource* GetResource(CUniqueID ResID, CFourCC type);
    CResource* GetResource(std::string res);
    void CacheResource(CResource *pRes);
    void DeleteResource(CUniqueID ResID);
};

extern CResCache gResCache;

#endif // CRESCACHE_H
