#ifndef CRESOURCEENTRY_H
#define CRESOURCEENTRY_H

#include "CVirtualDirectory.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/Flags.h>
#include <Common/types.h>

class CResource;
class CResourceStore;
class CDependencyTree;

enum EResEntryFlag
{
    eREF_NeedsRecook     = 0x1, // Resource has been updated but not recooked
    eREF_Transient       = 0x2, // Resource is transient (not part of a game project resource DB)
    eREF_HasThumbnail    = 0x4, // Resource has a unique thumbnail
    eREF_ThumbnailLoaded = 0x8, // Resource thumbnail is currently in memory
    // Flags that save to the cache file
    eREF_SavedFlags      = eREF_NeedsRecook | eREF_HasThumbnail
};
DECLARE_FLAGS(EResEntryFlag, FResEntryFlags)

class CResourceEntry
{
    CResource *mpResource;
    CResourceStore *mpStore;
    CDependencyTree *mpDependencies;
    CAssetID mID;
    EResType mType;
    EGame mGame;
    CVirtualDirectory *mpDirectory;
    TWideString mName;
    FResEntryFlags mFlags;

    mutable u64 mCachedSize;
    mutable TWideString mCachedUppercaseName; // This is used to speed up case-insensitive sorting and filtering.

public:
    CResourceEntry(CResourceStore *pStore, const CAssetID& rkID,
                   const TWideString& rkDir, const TWideString& rkFilename,
                   EResType Type, bool Transient = false);
    ~CResourceEntry();

    bool LoadCacheData();
    bool SaveCacheData();
    void UpdateDependencies();
    TWideString CacheDataPath(bool Relative = false) const;

    bool HasRawVersion() const;
    bool HasCookedVersion() const;
    TString RawAssetPath(bool Relative = false) const;
    TString CookedAssetPath(bool Relative = false) const;
    bool IsInDirectory(CVirtualDirectory *pDir) const;
    u64 Size() const;
    bool NeedsRecook() const;
    void SetGame(EGame NewGame);
    bool Save();
    CResource* Load();
    CResource* LoadCooked(IInputStream& rInput);
    bool Unload();
    void Move(const TWideString& rkDir, const TWideString& rkName);
    void AddToProject(const TWideString& rkDir, const TWideString& rkName);
    void RemoveFromProject();

    // Accessors
    void SetDirty()                                     { mFlags.SetFlag(eREF_NeedsRecook); }

    inline bool IsLoaded() const                        { return mpResource != nullptr; }
    inline CResource* Resource() const                  { return mpResource; }
    inline CDependencyTree* Dependencies() const        { return mpDependencies; }
    inline CAssetID ID() const                          { return mID; }
    inline EGame Game() const                           { return mGame; }
    inline CVirtualDirectory* Directory() const         { return mpDirectory; }
    inline TWideString Name() const                     { return mName; }
    inline const TWideString& UppercaseName() const     { return mCachedUppercaseName; }
    inline EResType ResourceType() const                { return mType; }
    inline bool IsTransient() const                     { return mFlags.HasFlag(eREF_Transient); }

protected:
    CResource* InternalLoad(IInputStream& rInput);
};

#endif // CRESOURCEENTRY_H
