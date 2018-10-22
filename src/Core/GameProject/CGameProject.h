#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CGameInfo.h"
#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/CAudioManager.h"
#include "Core/IProgressNotifier.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/types.h>
#include <Common/FileIO/CFileLock.h>

namespace nod { class DiscWii; }

class CGameProject
{
    TString mProjectName;
    EGame mGame;
    ERegion mRegion;
    TString mGameID;
    float mBuildVersion;

    TString mProjectRoot;
    std::vector<CPackage*> mPackages;
    CResourceStore *mpResourceStore;
    CGameInfo *mpGameInfo;
    CAudioManager *mpAudioManager;

    // Keep file handle open for the .prj file to prevent users from opening the same project
    // in multiple instances of PWE
    CFileLock mProjFileLock;

    enum EProjectVersion
    {
        eVer_Initial,

        eVer_Max,
        eVer_Current = eVer_Max - 1
    };

    // Private Constructor
    CGameProject()
        : mProjectName("Unnamed Project")
        , mGame(EGame::Invalid)
        , mRegion(ERegion::Unknown)
        , mGameID("000000")
        , mBuildVersion(0.f)
        , mpResourceStore(nullptr)
    {
        mpGameInfo = new CGameInfo();
        mpAudioManager = new CAudioManager(this);
    }

public:
    ~CGameProject();


    bool Save();
    bool Serialize(IArchive& rArc);
    bool BuildISO(const TString& rkIsoPath, IProgressNotifier *pProgress);
    bool MergeISO(const TString& rkIsoPath, nod::DiscWii *pOriginalIso, IProgressNotifier *pProgress);
    void GetWorldList(std::list<CAssetID>& rOut) const;
    CAssetID FindNamedResource(const TString& rkName) const;
    CPackage* FindPackage(const TString& rkName) const;

    // Static
    static CGameProject* CreateProjectForExport(
            const TString& rkProjRootDir,
            EGame Game,
            ERegion Region,
            const TString& rkGameID,
            float BuildVer
        );

    static CGameProject* LoadProject(const TString& rkProjPath, IProgressNotifier *pProgress);

    // Directory Handling
    inline TString ProjectRoot() const                      { return mProjectRoot; }
    inline TString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName, false) + ".prj"; }
    inline TString DiscDir(bool Relative) const             { return Relative ? "Disc/" : mProjectRoot + "Disc/"; }
    inline TString PackagesDir(bool Relative) const         { return Relative ? "Packages/" : mProjectRoot + "Packages/"; }
    inline TString ResourcesDir(bool Relative) const        { return Relative ? "Resources/" : mProjectRoot + "Resources/"; }

    // Disc Filesystem Management
    inline TString DiscFilesystemRoot(bool Relative) const  { return DiscDir(Relative) + (IsWiiBuild() ? "DATA/" : "") + "files/"; }

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
    inline bool IsTrilogy() const                       { return mGame <= EGame::Corruption && mBuildVersion >= 3.593f; }
    inline bool IsWiiDeAsobu() const                    { return mGame <= EGame::Corruption && mBuildVersion >= 3.570f && mBuildVersion < 3.593f; }
};

#endif // CGAMEPROJECT_H
