#ifndef CPACKAGE
#define CPACKAGE

#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>
#include "Core/IProgressNotifier.h"

class CGameProject;

enum class EPackageDefinitionVersion
{
    Initial,
    // Add new versions before this line

    Max,
    Current = Max - 1
};

struct SNamedResource
{
    TString Name;
    CAssetID ID;
    CFourCC Type;

    void Serialize(IArchive& rArc)
    {
        rArc << SerialParameter("Name", Name)
             << SerialParameter("ID", ID)
             << SerialParameter("Type", Type);
    }
};

class CPackage
{
    CGameProject *mpProject = nullptr;
    TString mPakName;
    TString mPakPath;
    std::vector<SNamedResource> mResources;
    bool mNeedsRecook = false;

    // Cached dependency list; used to figure out if a given resource is in this package
    mutable bool mCacheDirty = false;
    mutable std::set<CAssetID> mCachedDependencies;

public:
    CPackage() = default;
    CPackage(CGameProject *pProj, TString rkName, TString rkPath)
        : mpProject(pProj)
        , mPakName(std::move(rkName))
        , mPakPath(std::move(rkPath))
        , mCacheDirty(true)
    {}

    bool Load();
    bool Save();
    void Serialize(IArchive& rArc);
    void AddResource(const TString& rkName, const CAssetID& rkID, const CFourCC& rkType);
    void UpdateDependencyCache() const;
    void MarkDirty();

    void Cook(IProgressNotifier *pProgress);
    void CompareOriginalAssetList(const std::list<CAssetID>& rkNewList);
    bool ContainsAsset(const CAssetID& rkID) const;

    TString DefinitionPath(bool Relative) const;
    TString CookedPackagePath(bool Relative) const;

    // Accessors
    TString Name() const                                         { return mPakName; }
    TString Path() const                                         { return mPakPath; }
    CGameProject* Project() const                                { return mpProject; }
    size_t NumNamedResources() const                             { return mResources.size(); }
    const SNamedResource& NamedResourceByIndex(size_t Idx) const { return mResources[Idx]; }
    bool NeedsRecook() const                                     { return mNeedsRecook; }

    void SetPakName(TString NewName) { mPakName = std::move(NewName); }
};

#endif // CPACKAGE

