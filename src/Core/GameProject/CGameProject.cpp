#include "CGameProject.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include <tinyxml2.h>

using namespace tinyxml2;

void CGameProject::Load()
{
    TString ProjPath = ProjectPath().ToUTF8();
    XMLDocument Doc;
    Doc.LoadFile(*ProjPath);

    if (Doc.Error())
    {
        Log::Error("Unable to open game project at " + ProjPath);
        return;
    }

    XMLElement *pRoot = Doc.FirstChildElement("GameProject");
    //EProjectVersion Version = (EProjectVersion) TString(pRoot->Attribute("Version")).ToInt32(10);

    // Verify all elements are present
    XMLElement *pProjName = pRoot->FirstChildElement("Name");
    XMLElement *pGame = pRoot->FirstChildElement("Game");
    XMLElement *pResDB = pRoot->FirstChildElement("ResourceDB");
    XMLElement *pPackages = pRoot->FirstChildElement("Packages");

    if (!pProjName || !pGame || !pResDB || !pPackages)
    {
        TString MissingElem = pProjName ? (pGame ? (pResDB ? "Packages" : "ResourceDB") : "Game") : "Name";
        Log::Error("Unable to load game project at " + ProjPath + "; " + MissingElem + " element is missing");
        return;
    }

    mProjectName = pProjName->GetText();
    mGame = CMasterTemplate::FindGameForName( pGame->GetText() );
    mResourceDBPath = pResDB->GetText();

    // Load packages
    XMLElement *pPkgElem = pPackages->FirstChildElement("Package");

    while (pPkgElem)
    {
        pPkgElem = pPkgElem->NextSiblingElement("Package");
        TString Path = pPkgElem->Attribute("Path");

        if (Path.IsEmpty())
            Log::Error("Failed to load package in game project " + ProjPath + "; Path attribute is missing or empty");

        else
        {
            CPackage *pPackage = new CPackage(this, Path.GetFileName(false), TString(Path.GetFileDirectory()).ToUTF16());
            pPackage->Load();
            mPackages.push_back(pPackage);
        }
    }
}

void CGameProject::Save()
{
    XMLDocument Doc;

    XMLDeclaration *pDecl = Doc.NewDeclaration();
    Doc.LinkEndChild(pDecl);

    XMLElement *pRoot = Doc.NewElement("GameProject");
    pRoot->SetAttribute("Version", eVer_Current);
    Doc.LinkEndChild(pRoot);

    XMLElement *pProjName = Doc.NewElement("Name");
    pProjName->SetText(*mProjectName);
    pRoot->LinkEndChild(pProjName);

    XMLElement *pGame = Doc.NewElement("Game");
    pGame->SetText(*CMasterTemplate::FindGameName(mGame));
    pRoot->LinkEndChild(pGame);

    XMLElement *pResDB = Doc.NewElement("ResourceDB");
    pResDB->SetText(*mResourceDBPath.ToUTF8());
    pRoot->LinkEndChild(pResDB);

    XMLElement *pPackages = Doc.NewElement("Packages");
    pRoot->LinkEndChild(pPackages);

    for (u32 iPkg = 0; iPkg < mPackages.size(); iPkg++)
    {
        CPackage *pPackage = mPackages[iPkg];
        TWideString FullDefPath = pPackage->DefinitionPath(false);
        TWideString RelDefPath = FileUtil::MakeRelative(FullDefPath.GetFileDirectory(), PackagesDir(false));
        TString DefPath = TWideString(RelDefPath + FullDefPath.GetFileName()).ToUTF8();

        XMLElement *pPakElem = Doc.NewElement("Package");
        pPakElem->SetAttribute("Path", *DefPath);
        pPackages->LinkEndChild(pPakElem);
    }

    // Save Project
    TString ProjPath = ProjectPath().ToUTF8();
    XMLError Result = Doc.SaveFile(*ProjPath);

    if (Result != XML_SUCCESS)
        Log::Error("Failed to save game project at: " + ProjPath);
}
