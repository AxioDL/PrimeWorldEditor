#include "CGameProject.h"
#include "Core/Resource/Factory/CTemplateLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <Common/Serialization/XML.h>

CGameProject *CGameProject::mspActiveProject = nullptr;

CGameProject::~CGameProject()
{
    ASSERT(!mpResourceStore->IsDirty());

    if (IsActive())
        mspActiveProject = nullptr;

    delete mpAudioManager;
    delete mpGameInfo;
    delete mpResourceStore;
}

bool CGameProject::Load(const TWideString& rkPath)
{
    mProjectRoot = rkPath.GetFileDirectory();
    mProjectRoot.Replace(L"/", L"\\");

    TString ProjPath = rkPath.ToUTF8();
    CXMLReader Reader(ProjPath);
    mGame = Reader.Game();
    Serialize(Reader);
    CTemplateLoader::LoadGameTemplates(mGame);

    mpResourceStore->LoadResourceDatabase();
    mpGameInfo->LoadGameInfo(mGame);
    mpAudioManager->LoadAssets();
    return true;
}

void CGameProject::Save()
{
    TString ProjPath = ProjectPath().ToUTF8();
    CXMLWriter Writer(ProjPath, "GameProject", eVer_Current, mGame);
    Serialize(Writer);
}

void CGameProject::Serialize(IArchive& rArc)
{
    rArc << SERIAL("Name", mProjectName)
         << SERIAL("BuildVersion", mBuildVersion)
         << SERIAL("ResourceDB", mResourceDBPath);

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
        for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
            delete mPackages[iPkg];
        mPackages.clear();

        for (u32 iPkg = 0; iPkg < PackageList.size(); iPkg++)
        {
            const TWideString& rkPackagePath = PackageList[iPkg];
            TString PackageName = TWideString(rkPackagePath.GetFileName(false)).ToUTF8();
            TWideString PackageDir = rkPackagePath.GetFileDirectory();

            CPackage *pPackage = new CPackage(this, PackageName, PackageDir);
            pPackage->Load();
            mPackages.push_back(pPackage);
        }
    }
}

void CGameProject::SetActive()
{
    if (mspActiveProject != this)
    {
        mspActiveProject = this;
        gpResourceStore = mpResourceStore;
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
