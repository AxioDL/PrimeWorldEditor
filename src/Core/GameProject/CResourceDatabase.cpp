#include "CResourceDatabase.h"
#include "CGameProject.h"
#include "Core/Resource/CResCache.h"
#include <Common/AssertMacro.h>
#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <tinyxml2.h>

using namespace tinyxml2;

// ************ CResourceEntry ************
bool CResourceEntry::HasRawVersion() const
{
    return FileUtil::Exists(RawAssetPath());
}

bool CResourceEntry::HasCookedVersion() const
{
    return FileUtil::Exists(CookedAssetPath());
}

TString CResourceEntry::RawAssetPath() const
{
    TWideString Ext = GetRawExtension(mResourceType, mpDatabase->GameProject()->Game()).ToUTF16();
    return mpDatabase->GameProject()->ProjectRoot() + mFileDir + mFileName + L"." + Ext;
}

TString CResourceEntry::CookedAssetPath() const
{
    TWideString Ext = GetCookedExtension(mResourceType, mpDatabase->GameProject()->Game()).ToUTF16();
    return mpDatabase->GameProject()->CookedDir() + mFileDir + mFileName + L"." + Ext;
}

bool CResourceEntry::NeedsRecook() const
{
    // Assets that do not have a raw version can't be recooked since they will always just be saved cooked to begin with.
    // We will recook any asset where the raw version has been updated but not recooked yet. mNeedsRecook can also be
    // toggled to arbitrarily flag any asset for recook.
    if (!HasRawVersion()) return false;
    if (!HasCookedVersion()) return true;
    if (mNeedsRecook) return true;
    return (FileUtil::LastModifiedTime(CookedAssetPath()) < FileUtil::LastModifiedTime(RawAssetPath()));
}

// ************ CResourceDatabase ************
CResourceDatabase::CResourceDatabase(CGameProject *pProj)
    : mpProj(pProj)
{}

CResourceDatabase::~CResourceDatabase()
{
}

void CResourceDatabase::Load(const TString& rkPath)
{
    XMLDocument Doc;
    Doc.LoadFile(*rkPath);

    if (!Doc.Error())
    {
        XMLElement *pRoot = Doc.FirstChildElement("ResourceDatabase");
        //EVersion DatabaseVersion = (EVersion) TString(pRoot->Attribute("Version")).ToInt32(10); // Version currently unused

        XMLElement *pResources = pRoot->FirstChildElement("Resources");
        XMLElement *pRes = pResources->FirstChildElement("Resource");
        u32 ResIndex = 0;

        while (pRes)
        {
            XMLElement *pChild = pRes->FirstChildElement();

            bool HasID = false, HasType = false, HasDir = false, HasName = false;
            CUniqueID ID;
            EResType Type;
            TWideString FileDir;
            TWideString FileName;

            while (pChild)
            {
                TString NodeName = pChild->Name();

                if (NodeName == "ID")
                {
                    ID = CUniqueID::FromString(pChild->GetText());
                    HasID = true;
                }

                else if (NodeName == "Type")
                {
                    Type = CResource::ResTypeForExtension(pChild->GetText());
                    HasType = true;
                    ASSERT(Type != eInvalidResType);
                }

                else if (NodeName == "FileDir")
                {
                    FileDir = pChild->GetText();
                    HasDir = true;
                }

                else if (NodeName == "FileName")
                {
                    FileName = pChild->GetText();
                    HasName = true;
                }

                pChild = pChild->NextSiblingElement();
            }

            if (HasID && HasType && HasDir && HasName)
                RegisterResource(ID, FileDir, FileName, Type);
            else
                Log::Error("Error reading " + rkPath + ": Resource entry " + TString::FromInt32(ResIndex, 0, 10) + " is missing one or more components");

            ResIndex++;
            pRes = pRes->NextSiblingElement("Resource");
        }
    }
}

void CResourceDatabase::Save(const TString& rkPath) const
{
    XMLDocument Doc;

    XMLDeclaration *pDecl = Doc.NewDeclaration();
    Doc.LinkEndChild(pDecl);

    XMLElement *pRoot = Doc.NewElement("ResourceDatabase");
    pRoot->SetAttribute("Version", eVer_Current);
    Doc.LinkEndChild(pRoot);

    XMLElement *pResources = Doc.NewElement("Resources");
    pRoot->LinkEndChild(pResources);

    for (auto It = mResourceMap.begin(); It != mResourceMap.end(); It++)
    {
        CResourceEntry *pEntry = It->second;
        XMLElement *pRes = Doc.NewElement("Resource");
        pResources->LinkEndChild(pRes);

        XMLElement *pID = Doc.NewElement("ID");
        pID->SetText(*pEntry->ID().ToString());
        pRes->LinkEndChild(pID);

        XMLElement *pType = Doc.NewElement("Type");
        pType->SetText(*GetCookedExtension(pEntry->ResourceType(), mpProj->Game()));
        pRes->LinkEndChild(pType);

        XMLElement *pDir = Doc.NewElement("FileDir");
        pDir->SetText(*pEntry->FileDirectory());
        pRes->LinkEndChild(pDir);

        XMLElement *pName = Doc.NewElement("FileName");
        pName->SetText(*pEntry->FileName());
        pRes->LinkEndChild(pName);

        XMLElement *pRecook = Doc.NewElement("NeedsRecook");
        pRecook->SetText(pEntry->NeedsRecook() ? "true" : "false");
        pRes->LinkEndChild(pRecook);
    }

    Doc.SaveFile(*rkPath);
}

CResourceEntry* CResourceDatabase::FindResourceEntry(const CUniqueID& rkID) const
{
    auto Found = mResourceMap.find(rkID);
    if (Found == mResourceMap.end()) return nullptr;
    else return Found->second;
}

CResource* CResourceDatabase::LoadResource(const CUniqueID& rkID) const
{
    // todo: no handling for raw assets yet
    CResourceEntry *pEntry = FindResourceEntry(rkID);

    if (pEntry)
    {
        TString AssetPath = pEntry->CookedAssetPath();

        if (FileUtil::Exists(AssetPath))
            return gResCache.GetResource(pEntry->CookedAssetPath());
    }

    return nullptr;
}

bool CResourceDatabase::RegisterResource(const CUniqueID& rkID, const TWideString& rkDir, const TWideString& rkFileName, EResType Type)
{
    CResourceEntry *pEntry = FindResourceEntry(rkID);

    if (pEntry)
    {
        Log::Error("Attempted to register resource that's already tracked in the database: " + rkID.ToString() + " / " + rkDir.ToUTF8() + " / " + rkFileName.ToUTF8());
        return false;
    }

    else
    {
        pEntry = new CResourceEntry(this, rkID, rkDir, rkFileName.GetFileName(false), Type);

        if (!pEntry->HasCookedVersion() && !pEntry->HasRawVersion())
        {
            Log::Error("Attempted to register a resource that doesn't exist: " + rkID.ToString() + " | " + rkDir.ToUTF8() + " | " + rkFileName.ToUTF8());
            delete pEntry;
            return false;
        }

        else
        {
            mResourceMap[rkID] = pEntry;
            return true;
        }
    }
}
