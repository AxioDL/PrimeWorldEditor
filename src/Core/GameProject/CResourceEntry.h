#ifndef CRESOURCEENTRY_H
#define CRESOURCEENTRY_H

#include "CResourceStore.h"
#include "CVirtualDirectory.h"
#include "Core/Resource/CResTypeInfo.h"
#include "Core/Resource/EResourceType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/Flags.h>

class CResource;
class CGameProject;
class CDependencyTree;

enum class EResEntryFlag
{
    NeedsRecook         = 0x00000001, // Resource has been updated but not recooked
    IsBaseGameResource  = 0x00000002, // Resource is from the original game, not user-created
    Hidden              = 0x00000004, // Resource is hidden, doesn't show up in resource browser
    HasBeenModified     = 0x00000008, // Resource has been modified and resaved by the user
    AutoResName         = 0x00000010, // Resource name is auto-generated
    AutoResDir          = 0x00000020, // Resource directory name is auto-generated
    MarkedForDeletion   = 0x00000040, // Resource has been marked for deletion by the user
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

    mutable bool mMetadataDirty;
    mutable uint64 mCachedSize;
    mutable TString mCachedUppercaseName; // This is used to speed up case-insensitive sorting and filtering.

    // Private constructor
    CResourceEntry(CResourceStore *pStore);

public:
    static CResourceEntry* CreateNewResource(CResourceStore *pStore, const CAssetID& rkID,
                                             const TString& rkDir, const TString& rkName,
                                             EResourceType Type, bool ExistingResource = false);
    static CResourceEntry* BuildFromArchive(CResourceStore *pStore, IArchive& rArc);
    static CResourceEntry* BuildFromDirectory(CResourceStore *pStore, CResTypeInfo *pTypeInfo,
                                              const TString& rkDirPath, const TString& rkName);
    ~CResourceEntry();

    bool LoadMetadata();
    bool SaveMetadata(bool ForceSave = false);
    void SerializeEntryInfo(IArchive& rArc, bool MetadataOnly);
    void UpdateDependencies();

    bool HasRawVersion() const;
    bool HasCookedVersion() const;
    bool HasMetadataFile() const;
    TString RawAssetPath(bool Relative = false) const;
    TString RawExtension() const;
    TString CookedAssetPath(bool Relative = false) const;
    CFourCC CookedExtension() const;
    TString MetadataFilePath(bool Relative = false) const;
    bool IsInDirectory(CVirtualDirectory *pDir) const;
    uint64 Size() const;
    bool NeedsRecook() const;
    bool Save(bool SkipCacheSave = false, bool FlagForRecook = true);
    bool Cook();
    CResource* Load();
    CResource* LoadCooked(IInputStream& rInput);
    bool Unload();
    bool CanMoveTo(const TString& rkDir, const TString& rkName);
    bool MoveAndRename(const TString& rkDir, const TString& rkName, bool IsAutoGenDir = false, bool IsAutoGenName = false);
    bool Move(const TString& rkDir, bool IsAutoGenDir = false);
    bool Rename(const TString& rkName, bool IsAutoGenName = false);
    void MarkDeleted(bool InDeleted);

    CGameProject* Project() const;
    EGame Game() const;

    void SetFlag(EResEntryFlag Flag);
    void ClearFlag(EResEntryFlag Flag);

    // Accessors
    inline void SetFlagEnabled(EResEntryFlag Flag, bool Enabled)    { Enabled ? SetFlag(Flag) : ClearFlag(Flag); }

    inline void SetDirty()                          { SetFlag(EResEntryFlag::NeedsRecook); }
    inline void SetHidden(bool Hidden)              { SetFlagEnabled(EResEntryFlag::Hidden, Hidden); }
    inline bool HasFlag(EResEntryFlag Flag) const   { return mFlags.HasFlag(Flag); }
    inline bool IsHidden() const                    { return HasFlag(EResEntryFlag::Hidden); }
    inline bool IsMarkedForDeletion() const         { return HasFlag(EResEntryFlag::MarkedForDeletion); }

    inline bool IsLoaded() const                    { return mpResource != nullptr; }
    inline bool IsCategorized() const               { return mpDirectory && !mpDirectory->FullPath().CaseInsensitiveCompare( mpStore->DefaultResourceDirPath() ); }
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
    inline EResourceType ResourceType() const       { return mpTypeInfo->Type(); }

protected:
    CResource* InternalLoad(IInputStream& rInput);
};

#endif // CRESOURCEENTRY_H
