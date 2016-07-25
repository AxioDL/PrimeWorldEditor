#include "CPackage.h"
#include "CGameProject.h"
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <tinyxml2.h>

using namespace tinyxml2;

void CPackage::Load()
{
    TWideString DefPath = DefinitionPath(false);

    XMLDocument Doc;
    Doc.LoadFile(*DefPath.ToUTF8());

    if (Doc.Error())
    {
        Log::Error("Couldn't open pak definition at path: " + DefPath.ToUTF8());
        return;
    }

    XMLElement *pRoot = Doc.FirstChildElement("PackageDefinition");
    //EPackageDefinitionVersion Version = (EPackageDefinitionVersion) TString(pRoot->Attribute("Version")).ToInt32(10);

    XMLElement *pColElem = pRoot->FirstChildElement("ResourceCollection");

    while (pColElem)
    {
        CResourceCollection *pCollection = AddCollection( pColElem->Attribute("Name") );
        XMLElement *pResElem = pColElem->FirstChildElement("NamedResource");

        while (pResElem)
        {
            XMLElement *pNameElem = pResElem->FirstChildElement("Name");
            XMLElement *pIDElem = pResElem->FirstChildElement("ID");
            XMLElement *pTypeElem = pResElem->FirstChildElement("Type");

            if (!pIDElem || !pNameElem || !pTypeElem)
            {
                TString ElemName = (pNameElem ? (pIDElem ? "Type" : "ID") : "Name");
                Log::Error("Can't add named resource from pak definition at " + DefPath.ToUTF8() + "; " + ElemName + " element missing");
            }

            else
            {
                CAssetID ID = CAssetID::FromString(pIDElem->GetText());
                TString Name = pNameElem->GetText();
                CFourCC Type = CFourCC(pTypeElem->GetText());
                pCollection->AddResource(Name, ID, Type);
            }

            pResElem = pResElem->NextSiblingElement("NamedResource");
        }

        pColElem = pColElem->NextSiblingElement("ResourceCollection");
    }
}

void CPackage::Save()
{
    TWideString DefPath = DefinitionPath(false);
    FileUtil::CreateDirectory(DefPath.GetFileDirectory());

    // Write XML
    XMLDocument Doc;

    XMLDeclaration *pDecl = Doc.NewDeclaration();
    Doc.LinkEndChild(pDecl);

    XMLElement *pRoot = Doc.NewElement("PackageDefinition");
    pRoot->SetAttribute("Version", eVer_Current);
    Doc.LinkEndChild(pRoot);

    for (u32 iCol = 0; iCol < mCollections.size(); iCol++)
    {
        CResourceCollection *pCollection = mCollections[iCol];

        XMLElement *pColElem = Doc.NewElement("ResourceCollection");
        pColElem->SetAttribute("Name", *pCollection->Name());
        pRoot->LinkEndChild(pColElem);

        for (u32 iRes = 0; iRes < pCollection->NumResources(); iRes++)
        {
            const SNamedResource& rkRes = pCollection->ResourceByIndex(iRes);

            XMLElement *pResElem = Doc.NewElement("NamedResource");
            pColElem->LinkEndChild(pResElem);

            XMLElement *pName = Doc.NewElement("Name");
            pName->SetText(*rkRes.Name);
            pResElem->LinkEndChild(pName);

            XMLElement *pID = Doc.NewElement("ID");
            pID->SetText(*rkRes.ID.ToString());
            pResElem->LinkEndChild(pID);

            XMLElement *pType = Doc.NewElement("Type");
            pType->SetText(*rkRes.Type.ToString());
            pResElem->LinkEndChild(pType);
        }
    }

    XMLError Error = Doc.SaveFile(*DefPath.ToUTF8());

    if (Error != XML_SUCCESS)
        Log::Error("Failed to save pak definition at path: " + DefPath.ToUTF8());
}

TWideString CPackage::DefinitionPath(bool Relative) const
{
    return mpProject->PackagesDir(Relative) + mPakPath + mPakName.ToUTF16() + L".pkd";
}

TWideString CPackage::CookedPackagePath(bool Relative) const
{
    return mpProject->DiscDir(Relative) + mPakPath + mPakName.ToUTF16() + L".pak";
}

CResourceCollection* CPackage::AddCollection(const TString& rkName)
{
    CResourceCollection *pCollection = new CResourceCollection(rkName);
    mCollections.push_back(pCollection);
    return pCollection;
}

void CPackage::RemoveCollection(CResourceCollection *pCollection)
{
    for (u32 iCol = 0; iCol < mCollections.size(); iCol++)
    {
        if (mCollections[iCol] == pCollection)
        {
            RemoveCollection(iCol);
            break;
        }
    }
}

void CPackage::RemoveCollection(u32 Index)
{
    ASSERT(Index < mCollections.size());
    auto Iter = mCollections.begin() + Index;
    delete *Iter;
    mCollections.erase(Iter);
}
