#ifndef CRESOURCEENTRY_H
#define CRESOURCEENTRY_H

#include "CVirtualDirectory.h"
#include "Core/Resource/CResTypeInfo.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/Flags.h>
#include <Common/types.h>

class CResource;
class CResourceStore;
class CGameProject;
class CDependencyTree;

enum EResEntryFlag
{
    eREF_NeedsRecook     = 0x00000001, // Resource has been updated but not recooked
    eREF_Transient       = 0x00000002, // Resource is transient (not part of a game project resource DB)
    eREF_Hidden          = 0x00000004, // Resource is hidden, doesn't show up in resource browser
    eREF_HasBeenModified = 0x00000008, // Resource has been modified and resaved by the user
    eREF_IsUserResource  = 0x00000010, // Resource was created by the user (i.e. isn't a Retro Studios asset)
    // Flags that save to the cache file
    eREF_SavedFlags      = eREF_NeedsRecook | eREF_Hidden | eREF_HasBeenModified | eREF_IsUserResource
};
DECLARE_FLAGS(EResEntryFlag, FResEntryFlags)

class CResourceEntry
{
    CResource *mpResource;
    CResTypeInfo *mpTypeInfo;
    CResourceStore *mpStore;
    CDependencyTree *mpDependencies;
    CAssetID mID;
    CVirtualDirectory *mpDirectory;
    TString mName;
    FResEntryFlags mFlags;

    mutable u64 mCachedSize;
    mutable TString mCachedUppercaseName; // This is used to speed up case-insensitive sorting and filtering.

public:
    CResourceEntry(CResourceStore *pStore, const CAssetID& rkID,
                   const TString& rkDir, const TString& rkFilename,
                   EResType Type, bool Transient = false);
    ~CResourceEntry();

    void SerializeCacheData(IArchive& rArc);
    void UpdateDependencies();

    bool HasRawVersion() const;
    bool HasCookedVersion() const;
    TString RawAssetPath(bool Relative = false) const;
    TString RawExtension() const;
    TString CookedAssetPath(bool Relative = false) const;
    CFourCC CookedExtension() const;
    bool IsInDirectory(CVirtualDirectory *pDir) const;
    u64 Size() const;
    bool NeedsRecook() const;
    bool Save(bool SkipCacheSave = false);
    bool Cook();
    CResource* Load();
    CResource* LoadCooked(IInputStream& rInput);
    bool Unload();
    bool CanMoveTo(const TString& rkDir, const TString& rkName);
    bool Move(const TString& rkDir, const TString& rkName);
    void AddToProject(const TString& rkDir, const TString& rkName);
    void RemoveFromProject();
    CGameProject* Project() const;
    EGame Game() const;

    // Accessors
    void SetDirty()                                 { mFlags.SetFlag(eREF_NeedsRecook); }
    void SetHidden(bool Hidden)                     { Hidden ? mFlags.SetFlag(eREF_Hidden) : mFlags.ClearFlag(eREF_Hidden); }

    inline bool IsLoaded() const                    { return mpResource != nullptr; }
    inline bool IsCategorized() const               { return mpDirectory && mpDirectory->FullPath() != "Uncategorized/"; }
    inline bool IsNamed() const                     { return mName != mID.ToString(); }
    inline CResource* Resource() const              { return mpResource; }
    inline CResTypeInfo* TypeInfo() const           { return mpTypeInfo; }
    inline CResourceStore* ResourceStore() const    { return mpStore; }
    inline CDependencyTree* Dependencies() const    { return mpDependencies; }
    inline CAssetID ID() const                      { return mID; }
    inline CVirtualDirectory* Directory() const     { return mpDirectory; }
    inline TString DirectoryPath() const            { return mpDirectory->FullPath(); }
    inline TString Name() const                     { return mName; }
    inline const TString& UppercaseName() const     { return mCachedUppercaseName; }
    inline EResType ResourceType() const            { return mpTypeInfo->Type(); }
    inline bool IsTransient() const                 { return mFlags.HasFlag(eREF_Transient); }
    inline bool IsHidden() const                    { return mFlags.HasFlag(eREF_Hidden); }

protected:
    CResource* InternalLoad(IInputStream& rInput);
};

#endif // CRESOURCEENTRY_H
