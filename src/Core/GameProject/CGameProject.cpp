#include "CGameProject.h"
#include "IUIRelay.h"
#include "Core/Resource/Factory/CTemplateLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <Common/Serialization/XML.h>
#include <nod/nod.hpp>

CGameProject::~CGameProject()
{
    if (mpResourceStore)
    {
        ASSERT(!mpResourceStore->IsDirty());

        if (gpResourceStore == mpResourceStore)
            gpResourceStore = nullptr;
    }

    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
        delete mPackages[iPkg];

    delete mpAudioManager;
    delete mpGameInfo;
    delete mpResourceStore;
}

bool CGameProject::Save()
{
    mProjFileLock.Release();
    TString ProjPath = ProjectPath();
    CXMLWriter Writer(ProjPath, "GameProject", eVer_Current, mGame);
    Serialize(Writer);
    bool SaveSuccess = Writer.Save();
    mProjFileLock.Lock(*ProjPath);
    return SaveSuccess;
}

bool CGameProject::Serialize(IArchive& rArc)
{
    rArc << SERIAL("Name", mProjectName)
         << SERIAL("Region", mRegion)
         << SERIAL("GameID", mGameID)
         << SERIAL("BuildVersion", mBuildVersion)
         << SERIAL("DolPath", mDolPath)
         << SERIAL("ApploaderPath", mApploaderPath);

    if (rArc.Game() >= eCorruption)
        rArc << SERIAL("PartitionHeaderPath", mPartitionHeaderPath);

    if (!IsWiiBuild())
        rArc << SERIAL("FstAddress", mFilesystemAddress);

    rArc << SERIAL("ResourceDB", mResourceDBPath);

    // Serialize package list
    std::vector<TString> PackageList;

    if (!rArc.IsReader())
    {
        for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
            PackageList.push_back( mPackages[iPkg]->DefinitionPath(true) );
    }

    rArc << SERIAL_CONTAINER("Packages", PackageList, "Package");

    // Load packages
    if (rArc.IsReader())
    {
        ASSERT(mPackages.empty());

        for (u32 iPkg = 0; iPkg < PackageList.size(); iPkg++)
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

    if (IsWiiBuild())
    {
        Log::Error("Wii ISO building not supported!");
        return false;
    }

    else
    {
        auto ProgressCallback = [&](float ProgressPercent, const nod::SystemString& rkInfoString, size_t)
        {
            pProgress->Report((int) (ProgressPercent * 10000), 10000, TWideString(rkInfoString).ToUTF8());
        };

        nod::DiscBuilderGCN *pBuilder = new nod::DiscBuilderGCN(*rkIsoPath.ToUTF16(), *mGameID, *mProjectName, mFilesystemAddress, ProgressCallback);
        pProgress->SetTask(0, "Building " + rkIsoPath.GetFileName());

        TWideString ProjRoot = ProjectRoot().ToUTF16();
        TWideString DiscRoot = DiscDir(false).ToUTF16();
        TWideString DolPath = ProjRoot + mDolPath.ToUTF16();
        TWideString ApploaderPath = ProjRoot + mApploaderPath.ToUTF16();
        return pBuilder->buildFromDirectory(*DiscRoot, *DolPath, *ApploaderPath) == nod::EBuildResult::Success;
    }
}

void CGameProject::GetWorldList(std::list<CAssetID>& rOut) const
{
    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPkg = mPackages[iPkg];

        // Little workaround to fix some of Retro's paks having worlds listed in the wrong order...
        // Construct a sorted list of worlds in this package
        std::list<const SNamedResource*> PackageWorlds;

        for (u32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
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
    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPkg = mPackages[iPkg];

        for (u32 iRes = 0; iRes < pPkg->NumNamedResources(); iRes++)
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
    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
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
        float BuildVer,
        const TString& rkDolPath,
        const TString& rkApploaderPath,
        const TString& rkPartitionHeaderPath,
        u32 FstAddress
        )
{
    CGameProject *pProj = new CGameProject;
    pProj->mGame = Game;
    pProj->mRegion = Region;
    pProj->mGameID = rkGameID;
    pProj->mBuildVersion = BuildVer;
    pProj->mDolPath = rkDolPath;
    pProj->mApploaderPath = rkApploaderPath;
    pProj->mPartitionHeaderPath = rkPartitionHeaderPath;
    pProj->mFilesystemAddress = FstAddress;

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
        LoadSuccess = pProj->mpResourceStore->LoadResourceDatabase();

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

    CTemplateLoader::LoadGameTemplates(pProj->mGame);
    pProj->mProjFileLock.Lock(ProjPath);
    pProj->mpGameInfo->LoadGameInfo(pProj->mGame);
    pProj->mpAudioManager->LoadAssets();
    return pProj;
}
