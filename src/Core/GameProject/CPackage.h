#ifndef CPACKAGE
#define CPACKAGE

#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>
#include "Core/IProgressNotifier.h"

class CGameProject;

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
    CGameProject *mpProject;
    TString mPakName;
    TString mPakPath;
    std::vector<SNamedResource> mResources;
    bool mNeedsRecook;

    // Cached dependency list; used to figure out if a given resource is in this package
    mutable bool mCacheDirty;
    mutable std::set<CAssetID> mCachedDependencies;

    enum EPackageDefinitionVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

public:
    CPackage() {}

    CPackage(CGameProject *pProj, const TString& rkName, const TString& rkPath)
        : mpProject(pProj)
        , mPakName(rkName)
        , mPakPath(rkPath)
        , mNeedsRecook(false)
        , mCacheDirty(true)
    {}

    bool Load();
    bool Save();
    void Serialize(IArchive& rArc);
    void AddResource(const TString& rkName, const CAssetID& rkID, const CFourCC& rkType);
    void UpdateDependencyCache() const;

    void Cook(IProgressNotifier *pProgress);
    void CompareOriginalAssetList(const std::list<CAssetID>& rkNewList);
    bool ContainsAsset(const CAssetID& rkID) const;

    TString DefinitionPath(bool Relative) const;
    TString CookedPackagePath(bool Relative) const;

    // Accessors
    inline TString Name() const                                         { return mPakName; }
    inline TString Path() const                                         { return mPakPath; }
    inline CGameProject* Project() const                                { return mpProject; }
    inline u32 NumNamedResources() const                                { return mResources.size(); }
    inline const SNamedResource& NamedResourceByIndex(u32 Idx) const    { return mResources[Idx]; }
    inline bool NeedsRecook() const                                     { return mNeedsRecook; }

    inline void SetPakName(TString NewName) { mPakName = NewName; }
    inline void MarkDirty()                 { mNeedsRecook = true; Save(); UpdateDependencyCache(); }
};

#endif // CPACKAGE

