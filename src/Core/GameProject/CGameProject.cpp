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
    TString ProjPath = ProjectPath().ToUTF8();
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
            PackageList.push_back( mPackages[iPkg]->DefinitionPath(true).ToUTF8() );
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
                const TWideString& rkPackagePath = PackageList[iPkg];
                TString PackageName = TWideString(rkPackagePath.GetFileName(false)).ToUTF8();
                TWideString PackageDir = rkPackagePath.GetFileDirectory();

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

        for (u32 iCol = 0; iCol < pPkg->NumCollections(); iCol++)
        {
            CResourceCollection *pCol = pPkg->CollectionByIndex(iCol);

            for (u32 iRes = 0; iRes < pCol->NumResources(); iRes++)
            {
                const SNamedResource& rkRes = pCol->ResourceByIndex(iRes);

                if (rkRes.Type == "MLVL" && !rkRes.Name.EndsWith("NODEPEND"))
                    rOut.push_back(rkRes.ID);
            }
        }
    }
}

CAssetID CGameProject::FindNamedResource(const TString& rkName) const
{
    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPkg = mPackages[iPkg];

        for (u32 iCol = 0; iCol < pPkg->NumCollections(); iCol++)
        {
            CResourceCollection *pCol = pPkg->CollectionByIndex(iCol);

            for (u32 iRes = 0; iRes < pCol->NumResources(); iRes++)
            {
                const SNamedResource& rkRes = pCol->ResourceByIndex(iRes);

                if (rkRes.Name == rkName)
                    return rkRes.ID;
            }
        }
    }

    return CAssetID::InvalidID(mGame);
}

CGameProject* CGameProject::CreateProjectForExport(
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
    pProj->mProjectRoot.Replace(L"/", L"\\");
    pProj->mpResourceStore = new CResourceStore(pProj, pExporter, L"Content\\", L"Cooked\\", Game);
    pProj->mpGameInfo->LoadGameInfo(Game);
    pProj->mLoadSuccess = true;
    return pProj;
}

CGameProject* CGameProject::LoadProject(const TWideString& rkProjPath)
{
    CGameProject *pProj = new CGameProject;
    pProj->mProjectRoot = rkProjPath.GetFileDirectory();
    pProj->mProjectRoot.Replace(L"/", L"\\");

    TString ProjPath = rkProjPath.ToUTF8();
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
    pProj->mProjFileLock.Lock(*ProjPath);
    pProj->mpGameInfo->LoadGameInfo(pProj->mGame);
    pProj->mpAudioManager->LoadAssets();
    return pProj;
}
