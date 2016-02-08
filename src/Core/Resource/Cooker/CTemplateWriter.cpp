#include "CTemplateWriter.h"
#include "CAreaCooker.h"

#include <boost/filesystem.hpp>
#include <tinyxml2.h>

using namespace tinyxml2;

CTemplateWriter::CTemplateWriter()
{
}

void CTemplateWriter::SaveAllTemplates()
{
    // Create directory
    std::list<CMasterTemplate*> MasterList = CMasterTemplate::GetMasterList();
    TString Out = "../templates/";
    boost::filesystem::create_directory(Out.ToStdString());

    // Resave master templates
    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
        SaveGameTemplates(*it, Out);

    // Resave game list
    XMLDocument GameList;

    XMLDeclaration *pDecl = GameList.NewDeclaration();
    GameList.LinkEndChild(pDecl);

    XMLElement *pBase = GameList.NewElement("GameList");
    pBase->SetAttribute("version", 4);
    GameList.LinkEndChild(pBase);

    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        XMLElement *pGame = GameList.NewElement("game");
        pBase->LinkEndChild(pGame);

        XMLElement *pGameName = GameList.NewElement("name");
        pGameName->SetText(*pMaster->mGameName);
        pGame->LinkEndChild(pGameName);

        XMLElement *pAreaVersion = GameList.NewElement("mrea");
        u32 VersionNumber = CAreaCooker::GetMREAVersion(pMaster->GetGame());
        pAreaVersion->SetText(*TString::HexString(VersionNumber, true, true, 2));
        pGame->LinkEndChild(pAreaVersion);

        XMLElement *pTempPath = GameList.NewElement("master");
        pTempPath->SetText(*pMaster->mSourceFile);
        pGame->LinkEndChild(pTempPath);
    }

    TString GameListName = Out + "GameList.xml";
    GameList.SaveFile(*GameListName);
}

void CTemplateWriter::SaveGameTemplates(CMasterTemplate *pMaster, const TString& rkDir)
{
    // Create directory
    TString OutFile = rkDir + pMaster->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    boost::filesystem::create_directory(OutDir.ToStdString());

    // Resave script templates
    for (auto it = pMaster->mTemplates.begin(); it != pMaster->mTemplates.end(); it++)
        SaveScriptTemplate(it->second, OutDir);

    // Resave master template
    XMLDocument Master;

    XMLDeclaration *pDecl = Master.NewDeclaration();
    Master.LinkEndChild(pDecl);

    XMLElement *pBase = Master.NewElement("MasterTemplate");
    pBase->SetAttribute("version", 4);
    Master.LinkEndChild(pBase);

    // Write property list
    if (!pMaster->smPropertyNames.empty())
    {
        SavePropertyList(OutDir);

        XMLElement *pPropList = Master.NewElement("properties");
        pPropList->SetText("Properties.xml");
        pBase->LinkEndChild(pPropList);
    }

    // Write versions
    if (!pMaster->mGameVersions.empty())
    {
        XMLElement *pVersionsBlock = Master.NewElement("versions");
        pBase->LinkEndChild(pVersionsBlock);

        for (auto it = pMaster->mGameVersions.begin(); it != pMaster->mGameVersions.end(); it++)
        {
            XMLElement *pVersion = Master.NewElement("version");
            pVersion->SetText(*(*it));
            pBase->LinkEndChild(pVersion);
        }
    }

    // Write script objects
    XMLElement *pObjects = Master.NewElement("objects");
    pBase->LinkEndChild(pObjects);

    for (auto it = pMaster->mTemplates.begin(); it != pMaster->mTemplates.end(); it++)
    {
        u32 ObjID = (it->second)->ObjectID();

        TString StrID;
        if (ObjID <= 0xFF)
            StrID = TString::HexString(ObjID, true, true, 2);
        else
            StrID = CFourCC(ObjID).ToString();

        XMLElement *pObj = Master.NewElement("object");
        pObj->SetAttribute("ID", *StrID);
        pObj->SetAttribute("template", *(it->second)->mSourceFile);
        pObjects->LinkEndChild(pObj);
    }

    // Write script states/messages
    std::map<u32, TString> *pMaps[2] = { &pMaster->mStates, &pMaster->mMessages };
    TString Types[2] = { "state", "message" };

    for (u32 iScr = 0; iScr < 2; iScr++)
    {
        XMLElement *pElem = Master.NewElement(*(Types[iScr] + "s"));
        pBase->LinkEndChild(pElem);

        for (auto it = pMaps[iScr]->begin(); it != pMaps[iScr]->end(); it++)
        {
            TString ID;
            if (it->first <= 0xFF) ID = TString::HexString(it->first, true, true, 2);
            else ID = CFourCC(it->first).ToString();

            XMLElement *pSubElem = Master.NewElement(*Types[iScr]);
            pSubElem->SetAttribute("ID", *ID);
            pSubElem->SetAttribute("name", *(it->second));
            pElem->LinkEndChild(pSubElem);
        }
    }

    // Save file
    Master.SaveFile(*OutFile);
}

void CTemplateWriter::SavePropertyList(const TString& rkDir)
{
    // Create XML
    XMLDocument List;

    XMLDeclaration *pDecl = List.NewDeclaration();
    List.LinkEndChild(pDecl);

    XMLElement *pBase = List.NewElement("Properties");
    pBase->SetAttribute("version", 4);
    List.LinkEndChild(pBase);

    // Write properties
    for (auto it = CMasterTemplate::smPropertyNames.begin(); it != CMasterTemplate::smPropertyNames.end(); it++)
    {
        u32 ID = it->first;
        TString Name = it->second;

        XMLElement *pElem = List.NewElement("property");
        pElem->SetAttribute("ID", *TString::HexString(ID, true, true, 8));
        pElem->SetAttribute("name", *Name);
        pBase->LinkEndChild(pElem);
    }

    TString OutFile = rkDir + "Properties.xml";
    List.SaveFile(*OutFile);
}

void CTemplateWriter::SaveScriptTemplate(CScriptTemplate *pTemp, const TString& rkDir)
{
    // Create directory
    TString OutFile = rkDir + pTemp->mSourceFile;
    TString outDir = OutFile.GetFileDirectory();
    boost::filesystem::create_directory(*outDir);

    // Create new document
    XMLDocument ScriptXML;

    XMLDeclaration *pDecl = ScriptXML.NewDeclaration();
    ScriptXML.LinkEndChild(pDecl);

    // Base element
    XMLElement *pRoot = ScriptXML.NewElement("ScriptTemplate");
    pRoot->SetAttribute("version", 4);
    ScriptXML.LinkEndChild(pRoot);

    // Write object name
    XMLElement *pName = ScriptXML.NewElement("name");
    pName->SetText(*pTemp->Name());
    pRoot->LinkEndChild(pName);

    // Write properties
    SaveProperties(&ScriptXML, pRoot, pTemp->mpBaseStruct, pTemp->MasterTemplate(), rkDir);

    // Write editor properties
    XMLElement *pEditor = ScriptXML.NewElement("editor");
    pRoot->LinkEndChild(pEditor);

    // Editor Properties
    XMLElement *pEditorProperties = ScriptXML.NewElement("properties");
    pEditor->LinkEndChild(pEditorProperties);

    TString propNames[6] = {
        "InstanceName", "Position", "Rotation",
        "Scale", "Active", "LightParameters"
    };

    TIDString *pPropStrings[6] = {
        &pTemp->mNameIDString, &pTemp->mPositionIDString, &pTemp->mRotationIDString,
        &pTemp->mScaleIDString, &pTemp->mActiveIDString, &pTemp->mLightParametersIDString
    };

    for (u32 iProp = 0; iProp < 6; iProp++)
    {
        if (!pPropStrings[iProp]->IsEmpty())
        {
            XMLElement *pProperty = ScriptXML.NewElement("property");
            pProperty->SetAttribute("name", *propNames[iProp]);
            pProperty->SetAttribute("ID", **pPropStrings[iProp]);
            pEditorProperties->LinkEndChild(pProperty);
        }
    }

    // Editor Assets
    XMLElement *pAssets = ScriptXML.NewElement("assets");
    pEditor->LinkEndChild(pAssets);

    for (auto it = pTemp->mAssets.begin(); it != pTemp->mAssets.end(); it++)
    {
        TString Source = (it->AssetSource == CScriptTemplate::SEditorAsset::eFile ? "file" : "property");
        TString Type;

        switch (it->AssetType)
        {
        case CScriptTemplate::SEditorAsset::eModel:      Type = "model"; break;
        case CScriptTemplate::SEditorAsset::eAnimParams: Type = "animparams"; break;
        case CScriptTemplate::SEditorAsset::eBillboard:  Type = "billboard"; break;
        case CScriptTemplate::SEditorAsset::eCollision:  Type = "collision"; break;
        }

        s32 Force = -1;
        if (it->AssetSource == CScriptTemplate::SEditorAsset::eAnimParams)
            Force = it->ForceNodeIndex;

        XMLElement *pAsset = ScriptXML.NewElement(*Type);
        pAsset->SetAttribute("source", *Source);
        if (Force >= 0) pAsset->SetAttribute("force", std::to_string(Force).c_str());
        pAsset->SetText(*it->AssetLocation);
        pAssets->LinkEndChild(pAsset);
    }

    // Preview Scale
    if (pTemp->mPreviewScale != 1.f)
    {
        XMLElement *pPreviewScale = ScriptXML.NewElement("preview_scale");
        pPreviewScale->SetText(*TString::FromFloat(pTemp->mPreviewScale));
        pEditor->LinkEndChild(pPreviewScale);
    }

    // Rot/Scale Type
    XMLElement *pRotType = ScriptXML.NewElement("rotation_type");
    pEditor->LinkEndChild(pRotType);
    pRotType->SetText(pTemp->mRotationType == CScriptTemplate::eRotationEnabled ? "enabled" : "disabled");

    XMLElement *pScaleType = ScriptXML.NewElement("scale_type");
    pEditor->LinkEndChild(pScaleType);

    if (pTemp->mScaleType != CScriptTemplate::eScaleVolume)
        pScaleType->SetText(pTemp->mScaleType == CScriptTemplate::eScaleEnabled ? "enabled" : "disabled");

    else
    {
        pScaleType->SetText("volume");

        // Volume Preview
        XMLElement *pVolume = ScriptXML.NewElement("preview_volume");
        pEditor->LinkEndChild(pVolume);

        // Enum -> String conversion lambda to avoid redundant code
        auto GetVolumeString = [](EVolumeShape shape) -> TString
        {
            switch (shape)
            {
            case eBoxShape:            return "Box";
            case eAxisAlignedBoxShape: return "AxisAlignedBox";
            case eEllipsoidShape:      return "Ellipsoid";
            case eCylinderShape:       return "Cylinder";
            case eConditionalShape:    return "Conditional";
            default:                   return "INVALID";
            }
        };

        pVolume->SetAttribute("shape", *GetVolumeString(pTemp->mVolumeShape));

        if (pTemp->mVolumeScale != 1.f)
            pVolume->SetAttribute("scale", pTemp->mVolumeScale);

        if (pTemp->mVolumeShape == eConditionalShape)
        {
            pVolume->SetAttribute("propertyID", *pTemp->mVolumeConditionIDString);

            // Find conditional test property
            IPropertyTemplate *pProp = pTemp->mpBaseStruct->PropertyByIDString(pTemp->mVolumeConditionIDString);

            // Write conditions
            for (auto it = pTemp->mVolumeConditions.begin(); it != pTemp->mVolumeConditions.end(); it++)
            {
                // Value should be an integer, or a boolean condition?
                TString StrVal;

                if (pProp->Type() == eBoolProperty)
                    StrVal = (it->Value == 1 ? "true" : "false");
                else
                    StrVal = TString::HexString((u32) it->Value, true, true, (it->Value > 0xFF ? 8 : 2));

                XMLElement *pCondition = ScriptXML.NewElement("condition");
                pCondition->SetAttribute("value", *StrVal);
                pCondition->SetAttribute("shape", *GetVolumeString(it->Shape));
                if (it->Scale != 1.f) pCondition->SetAttribute("scale", it->Scale);
                pVolume->LinkEndChild(pCondition);
            }
        }
    }

    // Write to file
    ScriptXML.SaveFile(*OutFile);
}

void CTemplateWriter::SaveStructTemplate(CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& rkDir)
{
    // Create directory
    TString OutFile = rkDir + pTemp->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    TString Name = OutFile.GetFileName(false);
    boost::filesystem::create_directory(OutDir.ToStdString());

    // Create new document and write struct properties to it
    XMLDocument StructXML;

    XMLDeclaration *pDecl = StructXML.NewDeclaration();
    StructXML.LinkEndChild(pDecl);

    XMLElement *pRoot = StructXML.NewElement("struct");
    pRoot->SetAttribute("name", *Name);
    pRoot->SetAttribute("type", (pTemp->IsSingleProperty() ? "single" : "multi"));
    StructXML.LinkEndChild(pRoot);

    SaveProperties(&StructXML, pRoot, pTemp, pMaster, rkDir);
    StructXML.SaveFile(*OutFile);
}

void CTemplateWriter::SaveEnumTemplate(CEnumTemplate *pTemp, const TString& rkDir)
{
    // Create directory
    TString OutFile = rkDir + pTemp->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    TString Name = OutFile.GetFileName(false);
    boost::filesystem::create_directory(*OutDir);

    // Create new document and write enumerators to it
    XMLDocument EnumXML;

    XMLDeclaration *pDecl = EnumXML.NewDeclaration();
    EnumXML.LinkEndChild(pDecl);

    XMLElement *pRoot = EnumXML.NewElement("enum");
    pRoot->SetAttribute("name", *Name);
    EnumXML.LinkEndChild(pRoot);

    SaveEnumerators(&EnumXML, pRoot, pTemp);
    EnumXML.SaveFile(*OutFile);
}

void CTemplateWriter::SaveBitfieldTemplate(CBitfieldTemplate *pTemp, const TString& rkDir)
{
    // Create directory
    TString OutFile = rkDir + pTemp->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    TString Name = pTemp->mSourceFile.GetFileName(false);
    boost::filesystem::create_directory(*OutDir);

    // Create new document and write enumerators to it
    XMLDocument BitfieldXML;

    XMLDeclaration *pDecl = BitfieldXML.NewDeclaration();
    BitfieldXML.LinkEndChild(pDecl);

    XMLElement *pRoot = BitfieldXML.NewElement("bitfield");
    pRoot->SetAttribute("name", *Name);
    BitfieldXML.LinkEndChild(pRoot);

    SaveBitFlags(&BitfieldXML, pRoot, pTemp);
    BitfieldXML.SaveFile(*OutFile);
}

void CTemplateWriter::SaveProperties(XMLDocument *pDoc, XMLElement *pParent, CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& rkDir)
{
    // Create base element
    XMLElement *pPropsBlock = pDoc->NewElement("properties");
    pParent->LinkEndChild(pPropsBlock);

    for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
    {
        // Get ID
        IPropertyTemplate *pProp = pTemp->PropertyByIndex(iProp);
        u32 ID = pProp->PropertyID();
        TString StrID = TString::HexString(ID, true, true, (ID > 0xFF ? 8 : 2));

        // Create element
        XMLElement *pElem;

        if (pProp->Type() == eStructProperty)
            pElem = pDoc->NewElement("struct");
        else if (pProp->Type() == eEnumProperty)
            pElem = pDoc->NewElement("enum");
        else if (pProp->Type() == eBitfieldProperty)
            pElem = pDoc->NewElement("bitfield");
        else if (pProp->Type() == eArrayProperty)
            pElem = pDoc->NewElement("array");
        else
            pElem = pDoc->NewElement("property");

        // Set common property parameters, starting with ID
        pElem->SetAttribute("ID", *StrID);

        // Type
        if (pProp->Type() == eStructProperty)
            pElem->SetAttribute("type", (static_cast<CStructTemplate*>(pProp)->mIsSingleProperty ? "single" : "multi"));

        else if (TString(pElem->Name()) == "property")
            pElem->SetAttribute("type", *PropEnumToPropString(pProp->Type()));

        // Name
        TString Name = pProp->Name();

        if (pMaster->GetGame() >= eEchoesDemo)
        {
            TString MasterName = CMasterTemplate::GetPropertyName(ID);

            if (Name != MasterName)
                pElem->SetAttribute("name", *Name);
        }

        else
            pElem->SetAttribute("name", *Name);

        // Default
        if (pProp->CanHaveDefault())
        {
            XMLElement *pDefault = pDoc->NewElement("default");
            pDefault->SetText(*pProp->DefaultToString());
            pElem->LinkEndChild(pDefault);
        }

        // Range
        if (pProp->IsNumerical() && pProp->HasValidRange())
        {
            XMLElement *pRange = pDoc->NewElement("range");
            pRange->SetText(*pProp->RangeToString());
            pElem->LinkEndChild(pRange);
        }

        // Suffix
        if (pProp->IsNumerical())
        {
            TString Suffix = pProp->Suffix();

            if (!Suffix.IsEmpty())
            {
                XMLElement *pSuffix = pDoc->NewElement("suffix");
                pSuffix->SetText(*Suffix);
                pElem->LinkEndChild(pSuffix);
            }
        }

        // Cook Pref
        ECookPreference CookPref = pProp->CookPreference();

        if (CookPref != eNoCookPreference)
        {
            XMLElement *pCookPref = pDoc->NewElement("should_cook");
            pCookPref->SetText(CookPref == eAlwaysCook ? "always" : "never");
            pElem->LinkEndChild(pCookPref);
        }

        // File-specific parameters
        if (pProp->Type() == eFileProperty)
        {
            CFileTemplate *pFile = static_cast<CFileTemplate*>(pProp);
            const TStringList& rkExtensions = pFile->Extensions();
            TString ExtensionsString;

            for (auto it = rkExtensions.begin(); it != rkExtensions.end(); it++)
                ExtensionsString += *it + ",";

            ExtensionsString.ChopBack(1); // Remove extra comma
            pElem->SetAttribute("extensions", *ExtensionsString);
        }

        // Enum-specific parameters
        else if (pProp->Type() == eEnumProperty)
        {
            CEnumTemplate *pEnum = static_cast<CEnumTemplate*>(pProp);

            if (pEnum->mSourceFile.IsEmpty())
                SaveEnumerators(pDoc, pElem, pEnum);

            else
            {
                SaveEnumTemplate(pEnum, rkDir);
                pElem->SetAttribute("template", *pEnum->mSourceFile);
            }
        }

        // Bitfield-specific parameters
        else if (pProp->Type() == eBitfieldProperty)
        {
            CBitfieldTemplate *pBitfield = static_cast<CBitfieldTemplate*>(pProp);

            if (pBitfield->mSourceFile.IsEmpty())
                SaveBitFlags(pDoc, pElem, pBitfield);

            else
            {
                SaveBitfieldTemplate(pBitfield, rkDir);
                pElem->SetAttribute("template", *pBitfield->mSourceFile);
            }
        }

        // Struct/array-specific parameters
        if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
        {
            CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp);

            if (pStruct->mSourceFile.IsEmpty())
                SaveProperties(pDoc, pElem, pStruct, pMaster, rkDir);

            else
            {
                SaveStructTemplate(pStruct, pMaster, rkDir);
                pElem->SetAttribute("template", *pStruct->mSourceFile);
            }
        }
    }
}

void CTemplateWriter::SaveEnumerators(XMLDocument *pDoc, XMLElement *pParent, CEnumTemplate *pTemp)
{
    XMLElement *pEnumerators = pDoc->NewElement("enumerators");
    pParent->LinkEndChild(pEnumerators);

    for (u32 iEnum = 0; iEnum < pTemp->NumEnumerators(); iEnum++)
    {
        XMLElement *pElem = pDoc->NewElement("enumerator");
        u32 EnumerID = pTemp->EnumeratorID(iEnum);
        pElem->SetAttribute("ID", *TString::HexString(EnumerID, true, true, (EnumerID > 0xFF ? 8 : 0)));
        pElem->SetAttribute("name", *pTemp->EnumeratorName(iEnum));
        pEnumerators->LinkEndChild(pElem);
    }
}

void CTemplateWriter::SaveBitFlags(XMLDocument *pDoc, XMLElement *pParent, CBitfieldTemplate *pTemp)
{
    XMLElement *pFlags = pDoc->NewElement("flags");
    pParent->LinkEndChild(pFlags);

    for (u32 iFlag = 0; iFlag < pTemp->NumFlags(); iFlag++)
    {
        XMLElement *pElem = pDoc->NewElement("flag");
        pElem->SetAttribute("mask", *TString::HexString(pTemp->FlagMask(iFlag), true, true, 8));
        pElem->SetAttribute("name", *pTemp->FlagName(iFlag));
        pFlags->LinkEndChild(pElem);
    }
}
