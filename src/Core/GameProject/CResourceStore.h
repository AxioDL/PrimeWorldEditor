#ifndef CRESOURCEDATABASE_H
#define CRESOURCEDATABASE_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CFourCC.h>
#include <Common/CUniqueID.h>
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
    std::map<CUniqueID, CResourceEntry*> mResourceEntries;
    std::map<CUniqueID, CResourceEntry*> mLoadedResources;

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

    bool IsResourceRegistered(const CUniqueID& rkID) const;
    CResourceEntry* RegisterResource(const CUniqueID& rkID, EResType Type, const TWideString& rkDir, const TWideString& rkFileName);
    CResourceEntry* FindEntry(const CUniqueID& rkID) const;
    CResourceEntry* RegisterTransientResource(EResType Type, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");
    CResourceEntry* RegisterTransientResource(EResType Type, const CUniqueID& rkID, const TWideString& rkDir = L"", const TWideString& rkFileName = L"");

    CResource* LoadResource(const CUniqueID& rkID, const CFourCC& rkType);
    CResource* LoadResource(const TString& rkPath);
    void TrackLoadedResource(CResourceEntry *pEntry);
    CFourCC ResourceTypeByID(const CUniqueID& rkID, const TStringList& rkPossibleTypes) const;
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
