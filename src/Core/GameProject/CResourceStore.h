#ifndef CRESOURCEDATABASE_H
#define CRESOURCEDATABASE_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <map>
#include <set>

class CGameExporter;
class CGameProject;
class CResource;

class CResourceStore
{
    friend class CResourceIterator;

    CGameProject *mpProj;
    EGame mGame;
    CVirtualDirectory *mpDatabaseRoot;
    std::vector<CVirtualDirectory*> mTransientRoots;
    std::map<CAssetID, CResourceEntry*> mResourceEntries;
    std::map<CAssetID, CResourceEntry*> mLoadedResources;
    bool mDatabaseDirty;
    bool mCacheFileDirty;

    // Directory paths
    TWideString mDatabasePath;
    TWideString mDatabaseName;
    TWideString mRawDir;
    TWideString mCookedDir;
    TWideString mTransientLoadDir;

    // Game exporter currently in use - lets us load from paks being exported
    CGameExporter *mpExporter;

    enum EDatabaseVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CResourceStore(const TWideString& rkDatabasePath);
    CResourceStore(CGameProject *pProject, CGameExporter *pExporter, const TWideString& rkRawDir, const TWideString& rkCookedDir, EGame Game);
    CResourceStore(CGameProject *pProject);
    ~CResourceStore();
    void SerializeResourceDatabase(IArchive& rArc);
    void LoadResourceDatabase();
    void SaveResourceDatabase();
    void LoadCacheFile();
    void SaveCacheFile();
    void ConditionalSaveStore();
    void SetProject(CGameProject *pProj);
    void CloseProject();
    CVirtualDirectory* GetVirtualDirectory(const TWideString& rkPath, bool Transient, bool AllowCreate);
    void ConditionalDeleteDirectory(CVirtualDirectory *pDir);

    bool IsResourceRegistered(const CAssetID& rkID) const;
    CResourceEntry* RegisterResource(const CAssetID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkName);
    CResourceEntry* FindEntry(const CAssetID& rkID) const;
    CResourceEntry* FindEntry(const TWideString& rkPath) const;
    CResourceEntry* RegisterTransientResource(EResType Type, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");
    CResourceEntry* RegisterTransientResource(EResType Type, const CAssetID& rkID, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");

    CResource* LoadResource(const CAssetID& rkID, const CFourCC& rkType);
    CResource* LoadResource(const TWideString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    void DestroyUnreferencedResources();
    bool DeleteResourceEntry(CResourceEntry *pEntry);
    void SetTransientLoadDir(const TString& rkDir);

    void ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly);

    // Accessors
    inline CGameProject* Project() const                { return mpProj; }
    inline EGame Game() const                           { return mGame; }
    inline TWideString DatabaseRootPath() const         { return mDatabasePath; }
    inline TWideString RawDir(bool Relative) const      { return Relative ? mRawDir : mDatabasePath + mRawDir; }
    inline TWideString CookedDir(bool Relative) const   { return Relative ? mCookedDir : mDatabasePath + mCookedDir; }
    inline TWideString DatabasePath() const             { return DatabaseRootPath() + mDatabaseName; }
    inline TWideString CacheDataPath() const            { return DatabaseRootPath() + L"ResourceCacheData.rcd"; }
    inline CVirtualDirectory* RootDirectory() const     { return mpDatabaseRoot; }
    inline u32 NumTotalResources() const                { return mResourceEntries.size(); }
    inline u32 NumLoadedResources() const               { return mLoadedResources.size(); }
    inline bool IsDirty() const                         { return mDatabaseDirty || mCacheFileDirty; }

    inline void SetDatabaseDirty()                      { mDatabaseDirty = true; }
    inline void SetCacheDataDirty()                     { mCacheFileDirty = true; }
};

extern CResourceStore *gpResourceStore;
extern CResourceStore *gpEditorStore;

#endif // CRESOURCEDATABASE_H
