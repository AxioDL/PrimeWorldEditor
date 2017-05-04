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
    TString mDatabasePath;
    TString mDatabaseName;
    TString mRawDir;
    TString mCookedDir;
    TString mTransientLoadDir;

    enum EDatabaseVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CResourceStore(const TString& rkDatabasePath);
    CResourceStore(CGameProject *pProject, const TString& rkRawDir, const TString& rkCookedDir, EGame Game);
    CResourceStore(CGameProject *pProject);
    ~CResourceStore();
    void SerializeResourceDatabase(IArchive& rArc);
    bool LoadResourceDatabase();
    bool SaveResourceDatabase();
    bool LoadCacheFile();
    bool SaveCacheFile();
    void ConditionalSaveStore();
    void SetProject(CGameProject *pProj);
    void CloseProject();
    CVirtualDirectory* GetVirtualDirectory(const TString& rkPath, bool Transient, bool AllowCreate);
    void ConditionalDeleteDirectory(CVirtualDirectory *pDir);

    bool IsResourceRegistered(const CAssetID& rkID) const;
    CResourceEntry* RegisterResource(const CAssetID& rkID, EResType Type, const TString& rkDir, const TString& rkName);
    CResourceEntry* FindEntry(const CAssetID& rkID) const;
    CResourceEntry* FindEntry(const TString& rkPath) const;
    CResourceEntry* RegisterTransientResource(EResType Type, const TString& rkDir = "", const TString& rkFileName = "");
    CResourceEntry* RegisterTransientResource(EResType Type, const CAssetID& rkID, const TString& rkDir = "", const TString& rkFileName = "");

    CResource* LoadResource(const CAssetID& rkID, const CFourCC& rkType);
    CResource* LoadResource(const TString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    void DestroyUnreferencedResources();
    bool DeleteResourceEntry(CResourceEntry *pEntry);
    void SetTransientLoadDir(const TString& rkDir);

    void ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly);

    static bool IsValidResourcePath(const TString& rkPath, const TString& rkName);

    // Accessors
    inline CGameProject* Project() const            { return mpProj; }
    inline EGame Game() const                       { return mGame; }
    inline TString DatabaseRootPath() const         { return mDatabasePath; }
    inline TString RawDir(bool Relative) const      { return Relative ? mRawDir : mDatabasePath + mRawDir; }
    inline TString CookedDir(bool Relative) const   { return Relative ? mCookedDir : mDatabasePath + mCookedDir; }
    inline TString DatabasePath() const             { return DatabaseRootPath() + mDatabaseName; }
    inline TString CacheDataPath() const            { return DatabaseRootPath() + "ResourceCacheData.rcd"; }
    inline CVirtualDirectory* RootDirectory() const { return mpDatabaseRoot; }
    inline u32 NumTotalResources() const            { return mResourceEntries.size(); }
    inline u32 NumLoadedResources() const           { return mLoadedResources.size(); }
    inline bool IsDirty() const                     { return mDatabaseDirty || mCacheFileDirty; }

    inline void SetDatabaseDirty()                  { mDatabaseDirty = true; }
    inline void SetCacheDataDirty()                 { mCacheFileDirty = true; }
};

extern CResourceStore *gpResourceStore;
extern CResourceStore *gpEditorStore;

#endif // CRESOURCEDATABASE_H
