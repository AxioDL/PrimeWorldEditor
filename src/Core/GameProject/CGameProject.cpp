#include "CGameProject.h"
#include "IUIRelay.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <Common/Serialization/XML.h>
#include <nod/nod.hpp>

CGameProject::~CGameProject()
{
    if (mpResourceStore)
    {
        ASSERT(!mpResourceStore->IsCacheDirty());

        if (gpResourceStore == mpResourceStore)
            gpResourceStore = nullptr;
    }

    for (uint32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
        delete mPackages[iPkg];

    delete mpAudioManager;
    delete mpGameInfo;
    delete mpResourceStore;
}

bool CGameProject::Save()
{
    mProjFileLock.Release();
    TString ProjPath = ProjectPath();
    CXMLWriter Writer(ProjPath, "GameProject", (int) EProjectVersion::Current, mGame);
    Serialize(Writer);
    bool SaveSuccess = Writer.Save();
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
        for (uint32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
            PackageList.push_back( mPackages[iPkg]->DefinitionPath(true) );
    }

    rArc << SerialParameter("Packages", PackageList);

    // Load packages
    if (rArc.IsReader())
    {
        ASSERT(mPackages.empty());

        for (uint32 iPkg = 0; iPkg < PackageList.size(); iPkg++)
        {
            const TString& rkPackagePath = PackageList[iPkg];
            TString PackageName = rkPackagePath.GetFileName(false);
            TString PackageDir = rkPackagePath.GetFileDirectory();

            CPackage *pPackage = new CPackage(this, PackageName, PackageDir);
            bool PackageLoadSuccess = pPackage->Load();
            mPackages.push_back(pPackage);

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
    ASSERT( FileUtil::IsValidPath(rkIsoPath, false) );
    ASSERT( !IsWiiDeAsobu() && !IsTrilogy() );

    auto ProgressCallback = [&](float ProgressPercent, const nod::SystemStringView& rkInfoString, size_t)
    {
        pProgress->Report((int) (ProgressPercent * 10000), 10000, TWideString(rkInfoString.data()).ToUTF8());
    };

    pProgress->SetTask(0, "Building " + rkIsoPath.GetFileName());
    TWideString DiscRoot = DiscDir(false).ToUTF16();

    if (!IsWiiBuild())
    {
        nod::DiscBuilderGCN Builder(*rkIsoPath.ToUTF16(), ProgressCallback);
        return Builder.buildFromDirectory(*DiscRoot) == nod::EBuildResult::Success;
    }
    else
    {
        nod::DiscBuilderWii Builder(*rkIsoPath.ToUTF16(), IsTrilogy(), ProgressCallback);
        return Builder.buildFromDirectory(*DiscRoot) == nod::EBuildResult::Success;
    }
}

bool CGameProject::MergeISO(const TString& rkIsoPath, nod::DiscWii *pOriginalIso, IProgressNotifier *pProgress)
{
    ASSERT( FileUtil::IsValidPath(rkIsoPath, false) );
    ASSERT( IsWiiDeAsobu() || IsTrilogy() );
    ASSERT( pOriginalIso != nullptr );

    auto ProgressCallback = [&](float ProgressPercent, const nod::SystemStringView& rkInfoString, size_t)
    {
        pProgress->Report((int) (ProgressPercent * 10000), 10000, TWideString(rkInfoString.data()).ToUTF8());
    };

    pProgress->SetTask(0, "Building " + rkIsoPath.GetFileName());

    TWideString DiscRoot = DiscFilesystemRoot(false).ToUTF16();

    nod::DiscMergerWii Merger(*rkIsoPath.ToUTF16(), *pOriginalIso, IsTrilogy(), ProgressCallback);
    return Merger.mergeFromDirectory(*DiscRoot) == nod::EBuildResult::Success;
}

void CGameProject::GetWorldList(std::list<CAssetID>& rOut) const
{
    for (uint32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPkg = mPackages[iPkg];

        // Little workaround to fix some of Retro's paks having worlds listed in the wrong order...
        // Construct a sorted list of worlds in this package
        std::list<const SNamedResource*> PackageWorlds;

        for (uint32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);

            if (rkRes.Type == "MLVL" && !rkRes.Name.EndsWith("NODEPEND"))
                PackageWorlds.push_back(&rkRes);
        }

        PackageWorlds.sort([](const SNamedResource *pkLeft, const SNamedResource *pkRight) -> bool {
            return pkLeft->Name.ToUpper() < pkRight->Name.ToUpper();
        });

        // Add sorted worlds to the output world list
        for (auto Iter = PackageWorlds.begin(); Iter != PackageWorlds.end(); Iter++)
        {
            const SNamedResource *pkRes = *Iter;
            rOut.push_back(pkRes->ID);
        }
    }
}

CAssetID CGameProject::FindNamedResource(const TString& rkName) const
{
    for (uint32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPkg = mPackages[iPkg];

        for (uint32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
        {
            const SNamedResource& rkRes = pPkg->NamedResourceByIndex(iRes);

            if (rkRes.Name == rkName)
                return rkRes.ID;
        }
    }

    return CAssetID::InvalidID(mGame);
}

CPackage* CGameProject::FindPackage(const TString& rkName) const
{
    for (uint32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPackage = mPackages[iPkg];

        if (pPackage->Name() == rkName)
        {
            return pPackage;
        }
    }

    return nullptr;
}

CGameProject* CGameProject::CreateProjectForExport(
        const TString& rkProjRootDir,
        EGame Game,
        ERegion Region,
        const TString& rkGameID,
        float BuildVer
        )
{
    CGameProject *pProj = new CGameProject;
    pProj->mGame = Game;
    pProj->mRegion = Region;
    pProj->mGameID = rkGameID;
    pProj->mBuildVersion = BuildVer;

    pProj->mProjectRoot = rkProjRootDir;
    pProj->mProjectRoot.Replace("\\", "/");
    pProj->mpResourceStore = new CResourceStore(pProj);
    pProj->mpGameInfo->LoadGameInfo(Game);
    return pProj;
}

CGameProject* CGameProject::LoadProject(const TString& rkProjPath, IProgressNotifier *pProgress)
{
    // Init project
    CGameProject *pProj = new CGameProject;
    pProj->mProjectRoot = rkProjPath.GetFileDirectory();
    pProj->mProjectRoot.Replace("\\", "/");

    // Init progress
    pProgress->SetTask(0, "Loading project: " + rkProjPath.GetFileName());

    // Load main project file
    pProgress->Report("Loading project settings");
    bool LoadSuccess = false;

    TString ProjPath = rkProjPath;
    CXMLReader Reader(ProjPath);

    if (!Reader.IsValid())
    {
        delete pProj;
        return nullptr;
    }

    pProj->mGame = Reader.Game();

    if (pProj->Serialize(Reader))
    {
        // Load resource database
        pProgress->Report("Loading resource database");
        pProj->mpResourceStore = new CResourceStore(pProj);
        LoadSuccess = pProj->mpResourceStore->LoadDatabaseCache();

        // Validate resource database
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
    }

    if (!LoadSuccess)
    {
        delete pProj;
        return nullptr;
    }

    pProj->mProjFileLock.Lock(ProjPath);
    pProj->mpGameInfo->LoadGameInfo(pProj->mGame);
    pProj->mpAudioManager->LoadAssets();
    return pProj;
}
