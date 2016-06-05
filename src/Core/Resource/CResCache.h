#ifndef CRESCACHE_H
#define CRESCACHE_H

#include "CResource.h"
#include "Core/GameProject/CGameExporter.h"
#include <Common/types.h>
#include <Common/TString.h>
#include <unordered_map>

class CResCache
{
    std::unordered_map<u64, CResource*> mResourceCache;
    TString mResDir;
    CGameExporter *mpGameExporter;

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

    inline void SetGameExporter(CGameExporter *pExporter)   { mpGameExporter = pExporter; }

protected:
    CResource* InternalLoadResource(IInputStream& rInput, const CUniqueID& rkID, CFourCC Type);
};

extern CResCache gResCache;

#endif // CRESCACHE_H
