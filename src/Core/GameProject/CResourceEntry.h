#ifndef CRESOURCEENTRY_H
#define CRESOURCEENTRY_H

#include "CResourceStore.h"
#include "CVirtualDirectory.h"
#include "Core/Resource/CResTypeInfo.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/Flags.h>
#include <memory>

class CDependencyTree;
class CGameProject;
class CResource;
class IInputStream;

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
    std::unique_ptr<CResource> mpResource;
    CResTypeInfo *mpTypeInfo = nullptr;
    CResourceStore *mpStore;
    std::unique_ptr<CDependencyTree> mpDependencies;
    CAssetID mID;
    CVirtualDirectory *mpDirectory = nullptr;
    TString mName;
    FResEntryFlags mFlags;

    mutable bool mMetadataDirty = false;
    mutable uint64 mCachedSize = UINT64_MAX;
    mutable TString mCachedUppercaseName; // This is used to speed up case-insensitive sorting and filtering.

    // Private constructor
    explicit CResourceEntry(CResourceStore *pStore);

public:
    static std::unique_ptr<CResourceEntry> CreateNewResource(CResourceStore *pStore, const CAssetID& rkID,
                                                             const TString& rkDir, const TString& rkName,
                                                             EResourceType Type, bool ExistingResource = false);
    static std::unique_ptr<CResourceEntry> BuildFromArchive(CResourceStore *pStore, IArchive& rArc);
    static std::unique_ptr<CResourceEntry> BuildFromDirectory(CResourceStore *pStore, CResTypeInfo *pTypeInfo,
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
    void SetFlagEnabled(EResEntryFlag Flag, bool Enabled)    { Enabled ? SetFlag(Flag) : ClearFlag(Flag); }

    void SetDirty()                          { SetFlag(EResEntryFlag::NeedsRecook); }
    void SetHidden(bool Hidden)              { SetFlagEnabled(EResEntryFlag::Hidden, Hidden); }
    bool HasFlag(EResEntryFlag Flag) const   { return mFlags.HasFlag(Flag); }
    bool IsHidden() const                    { return HasFlag(EResEntryFlag::Hidden); }
    bool IsMarkedForDeletion() const         { return HasFlag(EResEntryFlag::MarkedForDeletion); }

    bool IsLoaded() const                    { return mpResource != nullptr; }
    bool IsCategorized() const               { return mpDirectory && !mpDirectory->FullPath().CaseInsensitiveCompare( mpStore->DefaultResourceDirPath() ); }
    bool IsNamed() const                     { return mName != mID.ToString(); }
    CResource* Resource() const              { return mpResource.get(); }
    CResTypeInfo* TypeInfo() const           { return mpTypeInfo; }
    CResourceStore* ResourceStore() const    { return mpStore; }
    CDependencyTree* Dependencies() const    { return mpDependencies.get(); }
    CAssetID ID() const                      { return mID; }
    CVirtualDirectory* Directory() const     { return mpDirectory; }
    TString DirectoryPath() const            { return mpDirectory->FullPath(); }
    TString Name() const                     { return mName; }
    const TString& UppercaseName() const     { return mCachedUppercaseName; }
    EResourceType ResourceType() const       { return mpTypeInfo->Type(); }

protected:
    CResource* InternalLoad(IInputStream& rInput);
};

#endif // CRESOURCEENTRY_H
