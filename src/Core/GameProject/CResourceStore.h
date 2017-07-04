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
    std::map<CAssetID, CResourceEntry*> mResourceEntries;
    std::map<CAssetID, CResourceEntry*> mLoadedResources;
    bool mDatabaseDirty;
    bool mCacheFileDirty;

    // Directory paths
    TString mDatabasePath;
    TString mDatabaseName;

    enum EDatabaseVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CResourceStore(const TString& rkDatabasePath);
    CResourceStore(CGameProject *pProject);
    ~CResourceStore();
    bool SerializeResourceDatabase(IArchive& rArc);
    bool LoadResourceDatabase();
    bool SaveResourceDatabase();
    bool LoadCacheFile();
    bool SaveCacheFile();
    void ConditionalSaveStore();
    void SetProject(CGameProject *pProj);
    void CloseProject();
    CVirtualDirectory* GetVirtualDirectory(const TString& rkPath, bool AllowCreate);
    void ConditionalDeleteDirectory(CVirtualDirectory *pDir, bool Recurse);

    bool IsResourceRegistered(const CAssetID& rkID) const;
    CResourceEntry* RegisterResource(const CAssetID& rkID, EResType Type, const TString& rkDir, const TString& rkName);
    CResourceEntry* FindEntry(const CAssetID& rkID) const;
    CResourceEntry* FindEntry(const TString& rkPath) const;
    bool AreAllEntriesValid() const;
    void ClearDatabase();
    void RebuildFromDirectory();

    template<typename ResType> ResType* LoadResource(const CAssetID& rkID)  { return static_cast<ResType*>(LoadResource(rkID, ResType::StaticType())); }
    CResource* LoadResource(const CAssetID& rkID);
    CResource* LoadResource(const CAssetID& rkID, EResType Type);
    CResource* LoadResource(const TString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    void DestroyUnreferencedResources();
    bool DeleteResourceEntry(CResourceEntry *pEntry);

    void ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly);

    static bool IsValidResourcePath(const TString& rkPath, const TString& rkName);

    // Accessors
    inline CGameProject* Project() const            { return mpProj; }
    inline EGame Game() const                       { return mGame; }
    inline TString DatabaseRootPath() const         { return mDatabasePath; }
    inline TString ResourcesDir() const             { return IsEditorStore() ? DatabaseRootPath() : DatabaseRootPath() + "Resources/"; }
    inline TString DatabasePath() const             { return DatabaseRootPath() + "ResourceDatabase.xml"; }
    inline TString CacheDataPath() const            { return DatabaseRootPath() + "ResourceCacheData.bin"; }
    inline CVirtualDirectory* RootDirectory() const { return mpDatabaseRoot; }
    inline u32 NumTotalResources() const            { return mResourceEntries.size(); }
    inline u32 NumLoadedResources() const           { return mLoadedResources.size(); }
    inline bool IsDirty() const                     { return mDatabaseDirty || mCacheFileDirty; }

    inline void SetDatabaseDirty()                  { mDatabaseDirty = true; }
    inline void SetCacheDataDirty()                 { mCacheFileDirty = true; }
    inline bool IsEditorStore() const               { return mpProj == nullptr; }
};

extern CResourceStore *gpResourceStore;
extern CResourceStore *gpEditorStore;

#endif // CRESOURCEDATABASE_H
