#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CGameInfo.h"
#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/CAudioManager.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <Common/FileIO/CFileLock.h>

class CGameProject
{
    TString mProjectName;
    EGame mGame;
    ERegion mRegion;
    TString mGameID;
    float mBuildVersion;
    TString mDolPath;
    TString mApploaderPath;
    TString mPartitionHeaderPath;
    u32 mFilesystemAddress;

    TString mProjectRoot;
    TString mResourceDBPath;
    std::vector<CPackage*> mPackages;
    CResourceStore *mpResourceStore;
    CGameInfo *mpGameInfo;
    CAudioManager *mpAudioManager;

    // Keep file handle open for the .prj file to prevent users from opening the same project
    // in multiple instances of PWE
    CFileLock mProjFileLock;
    bool mLoadSuccess;

    enum EProjectVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

    // Private Constructor
    CGameProject()
        : mProjectName("Unnamed Project")
        , mGame(eUnknownGame)
        , mRegion(eRegion_Unknown)
        , mGameID("000000")
        , mBuildVersion(0.f)
        , mResourceDBPath("ResourceDB.rdb")
        , mpResourceStore(nullptr)
        , mLoadSuccess(true)
    {
        mpGameInfo = new CGameInfo();
        mpAudioManager = new CAudioManager(this);
    }

public:
    ~CGameProject();

    bool Save();
    void Serialize(IArchive& rArc);
    void GetWorldList(std::list<CAssetID>& rOut) const;
    CAssetID FindNamedResource(const TString& rkName) const;
    CPackage* FindPackage(const TString& rkName) const;

    // Static
    static CGameProject* CreateProjectForExport(
            const TString& rkProjRootDir,
            EGame Game,
            ERegion Region,
            const TString& rkGameID,
            float BuildVer,
            const TString& rkDolPath,
            const TString& rkApploaderPath,
            const TString& rkPartitionHeaderPath,
            u32 FstAddress
        );

    static CGameProject* LoadProject(const TString& rkProjPath);

    // Directory Handling
    inline TString ProjectRoot() const                      { return mProjectRoot; }
    inline TString ResourceDBPath(bool Relative) const      { return Relative ? mResourceDBPath : mProjectRoot + mResourceDBPath; }
    inline TString DiscDir(bool Relative) const             { return Relative ? "Disc/" : mProjectRoot + "Disc/"; }
    inline TString CacheDir(bool Relative) const            { return Relative ? "Cache/" : mProjectRoot + "Cache/"; }
    inline TString PackagesDir(bool Relative) const         { return Relative ? "Packages/" : mProjectRoot + "Packages/"; }
    inline TString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName, false) + ".prj"; }
    inline TString ResourceCachePath(bool Relative) const   { return ResourceDBPath(Relative).GetFileDirectory() + "ResourceCacheData.rcd"; }

    // Accessors
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline TString Name() const                         { return mProjectName; }
    inline u32 NumPackages() const                      { return mPackages.size(); }
    inline CPackage* PackageByIndex(u32 Index) const    { return mPackages[Index]; }
    inline void AddPackage(CPackage *pPackage)          { mPackages.push_back(pPackage); }
    inline CResourceStore* ResourceStore() const        { return mpResourceStore; }
    inline CGameInfo* GameInfo() const                  { return mpGameInfo; }
    inline CAudioManager* AudioManager() const          { return mpAudioManager; }
    inline EGame Game() const                           { return mGame; }
    inline ERegion Region() const                       { return mRegion; }
    inline TString GameID() const                       { return mGameID; }
    inline float BuildVersion() const                   { return mBuildVersion; }
    inline bool IsWiiBuild() const                      { return mBuildVersion >= 3.f; }
};

#endif // CGAMEPROJECT_H
