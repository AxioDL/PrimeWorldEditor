#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/CAudioManager.h"
#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/types.h>

class CGameProject
{
    EGame mGame;
    TString mProjectName;
    TWideString mProjectRoot;
    TWideString mResourceDBPath;
    std::vector<CPackage*> mPackages;
    CResourceStore *mpResourceStore;
    CAudioManager mAudioManager;

    enum EProjectVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

    static CGameProject *mspActiveProject;

public:
    CGameProject()
        : mGame(eUnknownGame)
        , mProjectName("Unnamed Project")
        , mAudioManager(this)
    {
        mpResourceStore = new CResourceStore(this);
    }

    CGameProject(const TWideString& rkProjRootDir)
        : mGame(eUnknownGame)
        , mProjectName("Unnamed Project")
        , mProjectRoot(rkProjRootDir)
        , mResourceDBPath(L"ResourceDB.rdb")
        , mAudioManager(this)
    {
        mpResourceStore = new CResourceStore(this);
        mProjectRoot.Replace(L"/", L"\\");
    }

    ~CGameProject();

    bool Load(const TWideString& rkPath);
    void Save();
    void Serialize(IArchive& rArc);
    void SetActive();
    void GetWorldList(std::list<CAssetID>& rOut) const;
    CAssetID FindNamedResource(const TString& rkName) const;

    // Directory Handling
    inline TWideString ProjectRoot() const                      { return mProjectRoot; }
    inline TWideString ResourceDBPath(bool Relative) const      { return Relative ? mResourceDBPath : mProjectRoot + mResourceDBPath; }
    inline TWideString DiscDir(bool Relative) const             { return Relative ? L"Disc\\" : mProjectRoot + L"Disc\\"; }
    inline TWideString CacheDir(bool Relative) const            { return Relative ? L"Cache\\" : mProjectRoot + L"Cache\\"; }
    inline TWideString PackagesDir(bool Relative) const         { return Relative ? L"Packages\\" : mProjectRoot + L"Packages\\"; }
    inline TWideString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName.ToUTF16(), false) + L".prj"; }
    inline TWideString ResourceCachePath(bool Relative) const   { return ResourceDBPath(Relative).GetFileDirectory() + L"ResourceCacheData.rcd"; }

    // Accessors
    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline u32 NumPackages() const                      { return mPackages.size(); }
    inline CPackage* PackageByIndex(u32 Index) const    { return mPackages[Index]; }
    inline void AddPackage(CPackage *pPackage)          { mPackages.push_back(pPackage); }
    inline CResourceStore* ResourceStore() const        { return mpResourceStore; }
    inline CAudioManager* AudioManager()                { return &mAudioManager; }
    inline EGame Game() const                           { return mGame; }
    inline bool IsActive() const                        { return mspActiveProject == this; }

    static inline CGameProject* ActiveProject() { return mspActiveProject; }
};

extern CGameProject *gpProject;

#endif // CGAMEPROJECT_H
