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

class CGameProject
{
    TString mProjectName;
    EGame mGame;
    ERegion mRegion;
    TString mGameID;
    float mBuildVersion;
    TWideString mDolPath;
    TWideString mApploaderPath;
    TWideString mPartitionHeaderPath;
    u32 mFilesystemAddress;

    TWideString mProjectRoot;
    TWideString mResourceDBPath;
    std::vector<CPackage*> mPackages;
    CResourceStore *mpResourceStore;
    CGameInfo *mpGameInfo;
    CAudioManager *mpAudioManager;

    enum EProjectVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

    static CGameProject *mspActiveProject;

    // Private Constructor
    CGameProject()
        : mProjectName("Unnamed Project")
        , mGame(eUnknownGame)
        , mRegion(eRegion_Unknown)
        , mGameID("000000")
        , mBuildVersion(0.f)
        , mResourceDBPath(L"ResourceDB.rdb")
        , mpResourceStore(nullptr)
    {
        mpGameInfo = new CGameInfo();
        mpAudioManager = new CAudioManager(this);
    }

public:
    ~CGameProject();

    void Save();
    void Serialize(IArchive& rArc);
    void SetActive();
    void GetWorldList(std::list<CAssetID>& rOut) const;
    CAssetID FindNamedResource(const TString& rkName) const;

    // Static
    static CGameProject* CreateProjectForExport(
            CGameExporter *pExporter,
            const TWideString& rkProjRootDir,
            EGame Game,
            ERegion Region,
            const TString& rkGameID,
            float BuildVer,
            const TWideString& rkDolPath,
            const TWideString& rkApploaderPath,
            const TWideString& rkPartitionHeaderPath,
            u32 FstAddress
        );

    static CGameProject* LoadProject(const TWideString& rkProjPath);

    // Directory Handling
    inline TWideString ProjectRoot() const                      { return mProjectRoot; }
    inline TWideString ResourceDBPath(bool Relative) const      { return Relative ? mResourceDBPath : mProjectRoot + mResourceDBPath; }
    inline TWideString DiscDir(bool Relative) const             { return Relative ? L"Disc\\" : mProjectRoot + L"Disc\\"; }
    inline TWideString CacheDir(bool Relative) const            { return Relative ? L"Cache\\" : mProjectRoot + L"Cache\\"; }
    inline TWideString PackagesDir(bool Relative) const         { return Relative ? L"Packages\\" : mProjectRoot + L"Packages\\"; }
    inline TWideString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName.ToUTF16(), false) + L".prj"; }
    inline TWideString ResourceCachePath(bool Relative) const   { return ResourceDBPath(Relative).GetFileDirectory() + L"ResourceCacheData.rcd"; }

    // Accessors
    inline void SetProjectName(const TString& rkName)   { mProjectName = rkName; }

    inline u32 NumPackages() const                      { return mPackages.size(); }
    inline CPackage* PackageByIndex(u32 Index) const    { return mPackages[Index]; }
    inline void AddPackage(CPackage *pPackage)          { mPackages.push_back(pPackage); }
    inline CResourceStore* ResourceStore() const        { return mpResourceStore; }
    inline CGameInfo* GameInfo() const                  { return mpGameInfo; }
    inline CAudioManager* AudioManager() const          { return mpAudioManager; }
    inline EGame Game() const                           { return mGame; }
    inline float BuildVersion() const                   { return mBuildVersion; }
    inline bool IsActive() const                        { return mspActiveProject == this; }
    inline bool IsWiiBuild() const                      { return mBuildVersion >= 3.f; }

    static inline CGameProject* ActiveProject() { return mspActiveProject; }
};

#endif // CGAMEPROJECT_H
