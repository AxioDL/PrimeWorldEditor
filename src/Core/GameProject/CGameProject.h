#ifndef CGAMEPROJECT_H
#define CGAMEPROJECT_H

#include "CGameInfo.h"
#include "CPackage.h"
#include "CResourceStore.h"
#include "Core/CAudioManager.h"
#include "Core/IProgressNotifier.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Tweaks/CTweakManager.h"
#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/FileUtil.h>
#include <Common/TString.h>
#include <Common/FileIO/CFileLock.h>
#include <memory>
#include <string_view>

namespace nod { class DiscWii; }

enum class EProjectVersion
{
    Initial,
    RawStrings,
    // Add new versions before this line

    Max,
    Current = Max - 1
};

class CGameProject
{
    TString mProjectName{"Unnamed Project"};
    EGame mGame{EGame::Invalid};
    ERegion mRegion{ERegion::Unknown};
    TString mGameID{"000000"};
    float mBuildVersion = 0.f;

    TString mProjectRoot;
    std::vector<std::unique_ptr<CPackage>> mPackages;
    std::unique_ptr<CResourceStore> mpResourceStore;
    std::unique_ptr<CGameInfo> mpGameInfo = std::make_unique<CGameInfo>();
    std::unique_ptr<CAudioManager> mpAudioManager = std::make_unique<CAudioManager>(this);
    std::unique_ptr<CTweakManager> mpTweakManager = std::make_unique<CTweakManager>(this);

    // Keep file handle open for the .prj file to prevent users from opening the same project
    // in multiple instances of PWE
    CFileLock mProjFileLock;

    // Private Constructor
    CGameProject() = default;

public:
    ~CGameProject();

    bool Save();
    bool Serialize(IArchive& rArc);
    bool BuildISO(const TString& rkIsoPath, IProgressNotifier *pProgress);
    bool MergeISO(const TString& rkIsoPath, nod::DiscWii *pOriginalIso, IProgressNotifier *pProgress);
    void GetWorldList(std::list<CAssetID>& rOut) const;
    CAssetID FindNamedResource(std::string_view name) const;
    CPackage* FindPackage(std::string_view name) const;

    // Static
    static std::unique_ptr<CGameProject> CreateProjectForExport(
            const TString& rkProjRootDir,
            EGame Game,
            ERegion Region,
            const TString& rkGameID,
            float BuildVer
        );

    static std::unique_ptr<CGameProject> LoadProject(const TString& rkProjPath, IProgressNotifier *pProgress);

    // Directory Handling
    TString ProjectRoot() const                      { return mProjectRoot; }
    TString ProjectPath() const                      { return mProjectRoot + FileUtil::SanitizeName(mProjectName, false) + ".prj"; }
    TString HiddenFilesDir() const                   { return mProjectRoot + ".project/"; }
    TString DiscDir(bool Relative) const             { return Relative ? "Disc/" : mProjectRoot + "Disc/"; }
    TString PackagesDir(bool Relative) const         { return Relative ? "Packages/" : mProjectRoot + "Packages/"; }
    TString ResourcesDir(bool Relative) const        { return Relative ? "Resources/" : mProjectRoot + "Resources/"; }

    // Disc Filesystem Management
    TString DiscFilesystemRoot(bool Relative) const  { return DiscDir(Relative) + (IsWiiBuild() ? "DATA/" : "") + "files/"; }

    // Accessors
    void SetProjectName(TString name) { mProjectName = std::move(name); }

    TString Name() const                                 { return mProjectName; }
    size_t NumPackages() const                           { return mPackages.size(); }
    CPackage* PackageByIndex(size_t Index) const         { return mPackages[Index].get(); }
    void AddPackage(std::unique_ptr<CPackage>&& package) { mPackages.push_back(std::move(package)); }
    CResourceStore* ResourceStore() const                { return mpResourceStore.get(); }
    CGameInfo* GameInfo() const                          { return mpGameInfo.get(); }
    CAudioManager* AudioManager() const                  { return mpAudioManager.get(); }
    CTweakManager* TweakManager() const                  { return mpTweakManager.get(); }
    EGame Game() const                                   { return mGame; }
    ERegion Region() const                               { return mRegion; }
    TString GameID() const                               { return mGameID; }
    float BuildVersion() const                           { return mBuildVersion; }
    bool IsWiiBuild() const                              { return mBuildVersion >= 3.f; }
    bool IsTrilogy() const                               { return mGame <= EGame::Corruption && mBuildVersion >= 3.593f; }
    bool IsWiiDeAsobu() const                            { return mGame <= EGame::Corruption && mBuildVersion >= 3.570f && mBuildVersion < 3.593f; }
};

#endif // CGAMEPROJECT_H
