#include "CGameProject.h"
#include "Core/Resource/Factory/CTemplateLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <Common/Serialization/XML.h>

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

void CGameProject::Serialize(IArchive& rArc)
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

    // Packages
    std::vector<TString> PackageList;

    if (!rArc.IsReader())
    {
        for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
            PackageList.push_back( mPackages[iPkg]->DefinitionPath(true) );
    }

    rArc << SERIAL_CONTAINER("Packages", PackageList, "Package");

    if (rArc.IsReader())
    {
        // Load resource store
        ASSERT(mpResourceStore == nullptr);
        mpResourceStore = new CResourceStore(this);

        if (!mpResourceStore->LoadResourceDatabase())
            mLoadSuccess = false;

        else
        {
            // Load packages
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
                    mLoadSuccess = false;
                    break;
                }
            }
        }
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
    pProj->mpResourceStore = new CResourceStore(pProj, "Content/", "Cooked/", Game);
    pProj->mpGameInfo->LoadGameInfo(Game);
    pProj->mLoadSuccess = true;
    return pProj;
}

CGameProject* CGameProject::LoadProject(const TString& rkProjPath)
{
    CGameProject *pProj = new CGameProject;
    pProj->mProjectRoot = rkProjPath.GetFileDirectory();
    pProj->mProjectRoot.Replace("\\", "/");

    TString ProjPath = rkProjPath;
    CXMLReader Reader(ProjPath);

    if (!Reader.IsValid())
    {
        delete pProj;
        return nullptr;
    }

    pProj->mGame = Reader.Game();
    pProj->Serialize(Reader);

    if (!pProj->mLoadSuccess)
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
