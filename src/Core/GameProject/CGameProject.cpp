#include "CGameProject.h"
#include "CResourceIterator.h"
#include "IUIRelay.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <Common/Serialization/XML.h>
#include <nod/DiscGCN.hpp>
#include <nod/DiscWii.hpp>

#if NOD_UCS2
#define TStringToNodString(string) ToWChar(string)
#else
#define TStringToNodString(string) *string
#endif

CGameProject::~CGameProject()
{
    if (!mpResourceStore)
        return;

    ASSERT(!mpResourceStore->IsCacheDirty());

    if (gpResourceStore == mpResourceStore.get())
        gpResourceStore = nullptr;
}

bool CGameProject::Save()
{
    mProjFileLock.Release();
    const TString ProjPath = ProjectPath();
    CXMLWriter Writer(ProjPath, "GameProject", static_cast<int>(EProjectVersion::Current), mGame);
    Serialize(Writer);
    const bool SaveSuccess = Writer.Save();
    mProjFileLock.Lock(*ProjPath);
    return SaveSuccess;
}

bool CGameProject::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("Name", mProjectName)
         << SerialParameter("Region", mRegion)
         << SerialParameter("GameID", mGameID)
         << SerialParameter("BuildVersion", mBuildVersion);

    // Serialize package list
    std::vector<TString> PackageList;

    if (!rArc.IsReader())
    {
        for (auto& package : mPackages)
            PackageList.push_back(package->DefinitionPath(true));
    }

    rArc << SerialParameter("Packages", PackageList);

    // Load packages
    if (rArc.IsReader())
    {
        ASSERT(mPackages.empty());

        for (const TString& packagePath : PackageList)
        {
            TString PackageName = packagePath.GetFileName(false);
            TString PackageDir = packagePath.GetFileDirectory();

            auto pPackage = std::make_unique<CPackage>(this, std::move(PackageName), std::move(PackageDir));
            const bool PackageLoadSuccess = pPackage->Load();
            mPackages.push_back(std::move(pPackage));

            if (!PackageLoadSuccess)
            {
                return false;
            }
        }
    }

    return true;
}

bool CGameProject::BuildISO(const TString& rkIsoPath, IProgressNotifier *pProgress)
{
    ASSERT(FileUtil::IsValidPath(rkIsoPath, false));
    ASSERT(!IsWiiDeAsobu() && !IsTrilogy());

    const auto ProgressCallback = [&](float ProgressPercent, const nod::SystemStringView& rkInfoString, size_t)
    {
        pProgress->Report(static_cast<int>(ProgressPercent * 10000), 10000, nod::SystemUTF8Conv(rkInfoString).c_str());
    };

    pProgress->SetTask(0, "Building " + rkIsoPath.GetFileName());
    const TString DiscRoot = DiscDir(false);

    if (!IsWiiBuild())
    {
        nod::DiscBuilderGCN Builder(TStringToNodString(rkIsoPath), ProgressCallback);
        return Builder.buildFromDirectory(TStringToNodString(DiscRoot)) == nod::EBuildResult::Success;
    }
    else
    {
        nod::DiscBuilderWii Builder(TStringToNodString(rkIsoPath), IsTrilogy(), ProgressCallback);
        return Builder.buildFromDirectory(TStringToNodString(DiscRoot)) == nod::EBuildResult::Success;
    }
}

bool CGameProject::MergeISO(const TString& rkIsoPath, nod::DiscWii *pOriginalIso, IProgressNotifier *pProgress)
{
    ASSERT(FileUtil::IsValidPath(rkIsoPath, false));
    ASSERT(IsWiiDeAsobu() || IsTrilogy());
    ASSERT(pOriginalIso != nullptr);

    const auto ProgressCallback = [&](float ProgressPercent, const nod::SystemStringView& rkInfoString, size_t)
    {
        pProgress->Report(static_cast<int>(ProgressPercent * 10000), 10000, nod::SystemUTF8Conv(rkInfoString).c_str());
    };

    pProgress->SetTask(0, "Building " + rkIsoPath.GetFileName());

    const TString DiscRoot = DiscFilesystemRoot(false);

    nod::DiscMergerWii Merger(TStringToNodString(rkIsoPath), *pOriginalIso, IsTrilogy(), ProgressCallback);
    return Merger.mergeFromDirectory(TStringToNodString(DiscRoot)) == nod::EBuildResult::Success;
}

void CGameProject::GetWorldList(std::list<CAssetID>& rOut) const
{
    for (const auto& pPkg : mPackages)
    {
        // Little workaround to fix some of Retro's paks having worlds listed in the wrong order...
        // Construct a sorted list of worlds in this package
        std::list<const SNamedResource*> PackageWorlds;

        for (size_t iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);

            if (rkRes.Type == "MLVL" && !rkRes.Name.EndsWith("NODEPEND"))
                PackageWorlds.push_back(&rkRes);
        }

        PackageWorlds.sort([](const SNamedResource *pkLeft, const SNamedResource *pkRight) -> bool {
            return pkLeft->Name.ToUpper() < pkRight->Name.ToUpper();
        });

        // Add sorted worlds to the output world list
        for (const auto* res : PackageWorlds)
        {
            rOut.push_back(res->ID);
        }
    }
}

CAssetID CGameProject::FindNamedResource(std::string_view name) const
{
    for (const auto& pkg : mPackages)
    {
        for (size_t iRes = 0; iRes < pkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pkg->NamedResourceByIndex(iRes);

            if (rkRes.Name == name)
                return rkRes.ID;
        }
    }

    return CAssetID::InvalidID(mGame);
}

CPackage* CGameProject::FindPackage(std::string_view name) const
{
    const auto iter = std::find_if(mPackages.begin(), mPackages.end(),
                                   [name](const auto& package) { return package->Name() == name; });

    if (iter == mPackages.cend())
        return nullptr;

    return iter->get();
}

std::unique_ptr<CGameProject> CGameProject::CreateProjectForExport(
        const TString& rkProjRootDir,
        EGame Game,
        ERegion Region,
        const TString& rkGameID,
        float BuildVer
        )
{
    auto pProj = std::unique_ptr<CGameProject>(new CGameProject());
    pProj->mGame = Game;
    pProj->mRegion = Region;
    pProj->mGameID = rkGameID;
    pProj->mBuildVersion = BuildVer;

    pProj->mProjectRoot = rkProjRootDir;
    pProj->mProjectRoot.Replace("\\", "/");
    pProj->mpResourceStore = std::make_unique<CResourceStore>(pProj.get());
    pProj->mpGameInfo->LoadGameInfo(Game);
    return pProj;
}

std::unique_ptr<CGameProject> CGameProject::LoadProject(const TString& rkProjPath, IProgressNotifier *pProgress)
{
    // Init project
    auto pProj = std::unique_ptr<CGameProject>(new CGameProject());
    pProj->mProjectRoot = rkProjPath.GetFileDirectory();
    pProj->mProjectRoot.Replace("\\", "/");

    // Init progress
    pProgress->SetTask(0, "Loading project: " + rkProjPath.GetFileName());

    // Load main project file
    pProgress->Report("Loading project settings");
    bool LoadSuccess = false;

    const TString ProjPath = rkProjPath;
    CXMLReader Reader(ProjPath);

    if (!Reader.IsValid())
    {
        return nullptr;
    }

    pProj->mGame = Reader.Game();

    if (pProj->Serialize(Reader))
    {
        // Load resource database
        pProgress->Report("Loading resource database");
        pProj->mpResourceStore = std::make_unique<CResourceStore>(pProj.get());
        LoadSuccess = pProj->mpResourceStore->LoadDatabaseCache();

        // Removed database validation step. We used to do this on project load to make sure all data was correct, but this takes a long
        // time and significantly extends how long it takes to open a project. In actual practice, this isn't needed most of the time, and
        // in the odd case that it is needed, there is a button in the resource browser to rebuild the database. So in the interest of
        // making project startup faster, we no longer validate the database.
#if 0
        if (LoadSuccess)
        {
            pProgress->Report("Validating resource database");
            bool DatabaseIsValid = pProj->mpResourceStore->AreAllEntriesValid();

            // Resource database is corrupt. Ask the user if they want to rebuild it.
            if (!DatabaseIsValid)
            {
                bool ShouldRebuild = gpUIRelay->AskYesNoQuestion("Error", "The resource database is corrupt. Attempt to repair it?");

                if (ShouldRebuild)
                {
                    pProgress->Report("Repairing resource database");
                    pProj->mpResourceStore->RebuildFromDirectory();
                }
                else
                    LoadSuccess = false;
            }
        }
#endif
    }

    if (!LoadSuccess)
    {
        return nullptr;
    }

    pProj->mProjFileLock.Lock(ProjPath);
    pProj->mpGameInfo->LoadGameInfo(pProj->mGame);

    // Perform update
    if (Reader.FileVersion() < static_cast<uint16>(EProjectVersion::Current))
    {
        pProgress->Report("Updating project");

        CResourceStore* pOldStore = gpResourceStore;
        gpResourceStore = pProj->mpResourceStore.get();

        for (CResourceIterator It; It; ++It)
        {
            if (It->TypeInfo()->CanBeSerialized() && !It->HasRawVersion())
            {
                It->Save(true, false);

                // Touch the cooked file to update its last modified time.
                // This prevents PWE from erroneously thinking the cooked file is outdated
                // (due to the raw file we just made having a more recent last modified time)
                FileUtil::UpdateLastModifiedTime( It->CookedAssetPath() );
            }
        }

        pProj->mpResourceStore->ConditionalSaveStore();
        pProj->Save();

        gpResourceStore = pOldStore;
    }

    // Create hidden files directory, if needed
    const TString HiddenDir = pProj->HiddenFilesDir();

    if (!FileUtil::Exists(HiddenDir))
    {
        FileUtil::MakeDirectory(HiddenDir);
        FileUtil::MarkHidden(HiddenDir, true);
    }

    pProj->mpAudioManager->LoadAssets();
    pProj->mpTweakManager->LoadTweaks();
    return pProj;
}
