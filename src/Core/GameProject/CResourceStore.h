#ifndef CRESOURCESTORE_H
#define CRESOURCESTORE_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <map>
#include <memory>
#include <set>

class CGameExporter;
class CGameProject;
class CResource;

enum class EDatabaseVersion
{
    Initial,
    // Add new versions before this line

    Max,
    Current = Max - 1
};

class CResourceStore
{
    friend class CResourceIterator;

    CGameProject *mpProj = nullptr;
    EGame mGame{EGame::Prime};
    CVirtualDirectory *mpDatabaseRoot = nullptr;
    std::map<CAssetID, std::unique_ptr<CResourceEntry>> mResourceEntries;
    std::map<CAssetID, CResourceEntry*> mLoadedResources;
    bool mDatabaseCacheDirty = false;

    // Directory paths
    TString mDatabasePath;
    bool mDatabasePathExists = false;

public:
    explicit CResourceStore(const TString& rkDatabasePath);
    explicit CResourceStore(CGameProject *pProject);
    ~CResourceStore();
    bool SerializeDatabaseCache(IArchive& rArc);
    bool LoadDatabaseCache();
    bool SaveDatabaseCache();
    void ConditionalSaveStore();
    void SetProject(CGameProject *pProj);
    void CloseProject();

    CVirtualDirectory* GetVirtualDirectory(const TString& rkPath, bool AllowCreate);
    void CreateVirtualDirectory(const TString& rkPath);
    void ConditionalDeleteDirectory(CVirtualDirectory *pDir, bool Recurse);
    TString DefaultResourceDirPath() const;
    TString DeletedResourcePath() const;

    bool IsResourceRegistered(const CAssetID& rkID) const;
    CResourceEntry* CreateNewResource(const CAssetID& rkID, EResourceType Type, const TString& rkDir, const TString& rkName, bool ExistingResource = false);
    CResourceEntry* FindEntry(const CAssetID& rkID) const;
    CResourceEntry* FindEntry(const TString& rkPath) const;
    bool AreAllEntriesValid() const;
    void ClearDatabase();
    bool BuildFromDirectory(bool ShouldGenerateCacheFile);
    void RebuildFromDirectory();

    template<typename ResType> ResType* LoadResource(const CAssetID& rkID)  { return static_cast<ResType*>(LoadResource(rkID, ResType::StaticType())); }
    CResource* LoadResource(const CAssetID& rkID);
    CResource* LoadResource(const CAssetID& rkID, EResourceType Type);
    CResource* LoadResource(const TString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    void DestroyUnreferencedResources();
    bool DeleteResourceEntry(CResourceEntry *pEntry);

    void ImportNamesFromPakContentsTxt(const TString& rkTxtPath, bool UnnamedOnly);

    static bool IsValidResourcePath(const TString& rkPath, const TString& rkName);
    static TString StaticDefaultResourceDirPath(EGame Game);

    // Accessors
    CGameProject* Project() const            { return mpProj; }
    EGame Game() const                       { return mGame; }
    TString DatabaseRootPath() const         { return mDatabasePath; }
    bool DatabasePathExists() const          { return mDatabasePathExists; }
    TString ResourcesDir() const             { return IsEditorStore() ? DatabaseRootPath() : DatabaseRootPath() + "Resources/"; }
    TString DatabasePath() const             { return DatabaseRootPath() + "ResourceDatabaseCache.bin"; }
    CVirtualDirectory* RootDirectory() const { return mpDatabaseRoot; }
    uint32 NumTotalResources() const         { return mResourceEntries.size(); }
    uint32 NumLoadedResources() const        { return mLoadedResources.size(); }
    bool IsCacheDirty() const                { return mDatabaseCacheDirty; }

    void SetCacheDirty()                     { mDatabaseCacheDirty = true; }
    bool IsEditorStore() const               { return mpProj == nullptr; }
};

extern TString gDataDir;
extern bool gResourcesWritable;
extern bool gTemplatesWritable;
extern CResourceStore *gpResourceStore;
extern CResourceStore *gpEditorStore;

#endif // CRESOURCESTORE_H
