#ifndef CRESOURCEDATABASE_H
#define CRESOURCEDATABASE_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
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
    CVirtualDirectory *mpProjectRoot;
    std::vector<CVirtualDirectory*> mTransientRoots;
    std::map<CAssetID, CResourceEntry*> mResourceEntries;
    std::map<CAssetID, CResourceEntry*> mLoadedResources;

    // Directory to look for transient resources in
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
    CResourceStore();
    CResourceStore(CGameExporter *pExporter);
    ~CResourceStore();
    void LoadResourceDatabase(const TString& rkPath);
    void SaveResourceDatabase(const TString& rkPath) const;
    void SetActiveProject(CGameProject *pProj);
    void CloseActiveProject();
    CVirtualDirectory* GetVirtualDirectory(const TWideString& rkPath, bool Transient, bool AllowCreate);

    bool IsResourceRegistered(const CAssetID& rkID) const;
    CResourceEntry* RegisterResource(const CAssetID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkFileName);
    CResourceEntry* FindEntry(const CAssetID& rkID) const;
    CResourceEntry* RegisterTransientResource(EResType Type, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");
    CResourceEntry* RegisterTransientResource(EResType Type, const CAssetID& rkID, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");

    CResource* LoadResource(const CAssetID& rkID, const CFourCC& rkType);
    CResource* LoadResource(const TString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    CFourCC ResourceTypeByID(const CAssetID& rkID, const TStringList& rkPossibleTypes) const;
    void DestroyUnreferencedResources();
    bool DeleteResourceEntry(CResourceEntry *pEntry);
    void SetTransientLoadDir(const TString& rkDir);

    // Accessors
    inline CGameProject* ActiveProject() const      { return mpProj; }
    inline CVirtualDirectory* RootDirectory() const { return mpProjectRoot; }
    inline u32 NumTotalResources() const            { return mResourceEntries.size(); }
    inline u32 NumLoadedResources() const           { return mLoadedResources.size(); }
};

extern CResourceStore *gpResourceStore;

#endif // CRESOURCEDATABASE_H
