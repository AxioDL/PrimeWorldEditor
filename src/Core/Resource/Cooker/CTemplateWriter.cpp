#include "CTemplateWriter.h"
#include "CAreaCooker.h"

#include <boost/filesystem.hpp>
#include <tinyxml2.h>

using namespace tinyxml2;
TString CTemplateWriter::smTemplatesDir = "../templates/";

CTemplateWriter::CTemplateWriter()
{
}

void CTemplateWriter::SavePropertyTemplate(IPropertyTemplate *pTemp)
{
    // Check for a source file in the template's hierarchy; that indicates it's part of a struct template, not a script template
    TString SourceFile = pTemp->FindStructSource();

    // Struct
    if (!SourceFile.IsEmpty())
    {
        CMasterTemplate *pMaster = pTemp->MasterTemplate();
        auto StructIt = pMaster->mStructTemplates.find(SourceFile);

        if (StructIt != pMaster->mStructTemplates.end())
        {
            CStructTemplate *pStruct = StructIt->second;
            CTemplateWriter::SaveStructTemplate(pStruct);
        }
    }

    // Script
    else if (pTemp->ScriptTemplate())
        CTemplateWriter::SaveScriptTemplate(pTemp->ScriptTemplate());

    // Error
    else
        Log::Error("Couldn't save property template " + pTemp->IDString(true) + "; no struct template source path or script template found");
}

void CTemplateWriter::SaveAllTemplates()
{
    // Create directory
    std::list<CMasterTemplate*> MasterList = CMasterTemplate::MasterList();
    boost::filesystem::create_directory(smTemplatesDir.ToStdString());

    // Resave property list
    SavePropertyList();

    // Resave master templates
    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
        SaveGameTemplates(*it);

    // Resave game list
    XMLDocument GameList;

    XMLDeclaration *pDecl = GameList.NewDeclaration();
    GameList.LinkEndChild(pDecl);

    XMLElement *pBase = GameList.NewElement("GameList");
    pBase->SetAttribute("version", 4);
    GameList.LinkEndChild(pBase);

    XMLElement *pProperties = GameList.NewElement("properties");
    pProperties->SetText("Properties.xml");
    pBase->LinkEndChild(pProperties);

    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        XMLElement *pGame = GameList.NewElement("game");
        pBase->LinkEndChild(pGame);

        XMLElement *pGameName = GameList.NewElement("name");
        pGameName->SetText(*pMaster->mGameName);
        pGame->LinkEndChild(pGameName);

        XMLElement *pAreaVersion = GameList.NewElement("mrea");
        u32 VersionNumber = CAreaCooker::GetMREAVersion(pMaster->Game());
        pAreaVersion->SetText(*TString::HexString(VersionNumber, 2));
        pGame->LinkEndChild(pAreaVersion);

        XMLElement *pTempPath = GameList.NewElement("master");
        pTempPath->SetText(*pMaster->mSourceFile);
        pGame->LinkEndChild(pTempPath);
    }

    TString GameListName = smTemplatesDir + "GameList.xml";
    GameList.SaveFile(*GameListName);
}

void CTemplateWriter::SaveGameTemplates(CMasterTemplate *pMaster)
{
    // Create directory
    TString OutFile = smTemplatesDir + pMaster->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    boost::filesystem::create_directory(OutDir.ToStdString());

    // Resave script templates
    for (auto it = pMaster->mTemplates.begin(); it != pMaster->mTemplates.end(); it++)
        SaveScriptTemplate(it->second);

    // Resave struct templates
    for (auto it = pMaster->mStructTemplates.begin(); it != pMaster->mStructTemplates.end(); it++)
        SaveStructTemplate(it->second);

    // Resave master template
    XMLDocument Master;

    XMLDeclaration *pDecl = Master.NewDeclaration();
    Master.LinkEndChild(pDecl);

    XMLElement *pBase = Master.NewElement("MasterTemplate");
    pBase->SetAttribute("version", 4);
    Master.LinkEndChild(pBase);

    // Write versions
    if (!pMaster->mGameVersions.empty())
    {
        XMLElement *pVersionsBlock = Master.NewElement("versions");
        pBase->LinkEndChild(pVersionsBlock);

        for (auto it = pMaster->mGameVersions.begin(); it != pMaster->mGameVersions.end(); it++)
        {
            XMLElement *pVersion = Master.NewElement("version");
            pVersion->SetText(*(*it));
            pVersionsBlock->LinkEndChild(pVersion);
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
            StrID = TString::HexString(ObjID, 2);
        else
            StrID = CFourCC(ObjID).ToString();

        XMLElement *pObj = Master.NewElement("object");
        pObj->SetAttribute("ID", *StrID);
        pObj->SetAttribute("template", *(it->second)->mSourceFile);
        pObjects->LinkEndChild(pObj);
    }

    // Write script states/messages
    for (u32 iType = 0; iType < 2; iType++)
    {
        TString Type = (iType == 0 ? "state" : "message");
        XMLElement *pElem = Master.NewElement(*(Type + "s"));
        pBase->LinkEndChild(pElem);

        u32 Num = (iType == 0 ? pMaster->NumStates() : pMaster->NumMessages());

        for (u32 iScr = 0; iScr < Num; iScr++)
        {
            u32 ID;
            TString Name;

            if (iType == 0)
            {
                SState State = pMaster->StateByIndex(iScr);
                ID = State.ID;
                Name = State.Name;
            }
            else
            {
                SMessage Message = pMaster->MessageByIndex(iScr);
                ID = Message.ID;
                Name = Message.Name;
            }

            TString StrID;
            if (ID <= 0xFF) StrID = TString::HexString(ID, 2);
            else StrID = CFourCC(ID).ToString();

            XMLElement *pSubElem = Master.NewElement(*Type);
            pSubElem->SetAttribute("ID", *StrID);
            pSubElem->SetAttribute("name", *Name);
            pElem->LinkEndChild(pSubElem);
        }
    }

    // Save file
    Master.SaveFile(*OutFile);
}

void CTemplateWriter::SavePropertyList()
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
        pElem->SetAttribute("ID", *TString::HexString(ID));
        pElem->SetAttribute("name", *Name);
        pBase->LinkEndChild(pElem);
    }

    TString OutFile = smTemplatesDir + "Properties.xml";
    List.SaveFile(*OutFile);
}

void CTemplateWriter::SaveScriptTemplate(CScriptTemplate *pTemp)
{
    CMasterTemplate *pMaster = pTemp->MasterTemplate();

    // Create directory
    TString OutFile = smTemplatesDir + pMaster->GetDirectory() + pTemp->mSourceFile;
    TString OutDir = OutFile.GetFileDirectory();
    boost::filesystem::create_directory(*OutDir);

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
    SaveProperties(&ScriptXML, pRoot, pTemp->mpBaseStruct);

    // States/Messages [todo]
    XMLElement *pStates = ScriptXML.NewElement("states");
    pRoot->LinkEndChild(pStates);

    XMLElement *pMessages = ScriptXML.NewElement("messages");
    pRoot->LinkEndChild(pMessages);

    // Write editor properties
    XMLElement *pEditor = ScriptXML.NewElement("editor");
    pRoot->LinkEndChild(pEditor);

    // Editor Properties
    XMLElement *pEditorProperties = ScriptXML.NewElement("properties");
    pEditor->LinkEndChild(pEditorProperties);

    TString PropNames[6] = {
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
            pProperty->SetAttribute("name", *PropNames[iProp]);
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
        if (it->AssetType == CScriptTemplate::SEditorAsset::eAnimParams)
            Force = it->ForceNodeIndex;

        XMLElement *pAsset = ScriptXML.NewElement(*Type);
        pAsset->SetAttribute("source", *Source);
        if (Force >= 0) pAsset->SetAttribute("force", *TString::FromInt32(Force, 0, 10));
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
                // Value should be an integer, or a boolean condition
                TString StrVal;

                if (pProp->Type() == eBoolProperty)
                    StrVal = (it->Value == 1 ? "true" : "false");
                else
                    StrVal = TString::HexString((u32) it->Value, (it->Value > 0xFF ? 8 : 2));

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

void CTemplateWriter::SaveStructTemplate(CStructTemplate *pTemp)
{
    // Create directory
    CMasterTemplate *pMaster = pTemp->MasterTemplate();
    TString OutFile = smTemplatesDir + pMaster->GetDirectory() + pTemp->mSourceFile;
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

    SaveProperties(&StructXML, pRoot, pTemp);
    StructXML.SaveFile(*OutFile);
}

void CTemplateWriter::SaveEnumTemplate(CEnumTemplate *pTemp)
{
    // Create directory
    CMasterTemplate *pMaster = pTemp->MasterTemplate();
    TString OutFile = smTemplatesDir + pMaster->GetDirectory() + pTemp->mSourceFile;
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

void CTemplateWriter::SaveBitfieldTemplate(CBitfieldTemplate *pTemp)
{
    // Create directory
    CMasterTemplate *pMaster = pTemp->MasterTemplate();
    TString OutFile = smTemplatesDir + pMaster->GetDirectory() + pTemp->mSourceFile;
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

void CTemplateWriter::SaveProperties(XMLDocument *pDoc, XMLElement *pParent, CStructTemplate *pTemp)
{
    // Create base element
    XMLElement *pPropsBlock = pDoc->NewElement("properties");
    pParent->LinkEndChild(pPropsBlock);

    for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
    {
        // Get ID
        IPropertyTemplate *pProp = pTemp->PropertyByIndex(iProp);
        u32 ID = pProp->PropertyID();
        TString StrID = TString::HexString(ID, (ID > 0xFF ? 8 : 2));

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

        pPropsBlock->LinkEndChild(pElem);

        // Set common property parameters, starting with ID
        pElem->SetAttribute("ID", *StrID);

        // Name
        TString Name = pProp->Name();

        if (pProp->Game() >= eEchoesDemo && ID > 0xFF)
        {
            TString MasterName = CMasterTemplate::PropertyName(ID);

            if (Name != MasterName)
                pElem->SetAttribute("name", *Name);
        }

        else
            pElem->SetAttribute("name", *Name);

        // Type
        if (pProp->Type() == eStructProperty)
        {
            CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp);

            if (pStruct->mSourceFile.IsEmpty())
                pElem->SetAttribute("type", (pStruct->mIsSingleProperty ? "single" : "multi"));
        }

        else if (TString(pElem->Name()) == "property")
            pElem->SetAttribute("type", *PropEnumToPropString(pProp->Type()));

        // Versions
        CMasterTemplate *pMaster = pProp->MasterTemplate();
        u32 NumVersions = pProp->mAllowedVersions.size();

        if (NumVersions > 0 && NumVersions != pMaster->mGameVersions.size())
        {
            XMLElement *pVersions = pDoc->NewElement("versions");
            pElem->LinkEndChild(pVersions);

            for (u32 iVer = 0; iVer < pMaster->mGameVersions.size(); iVer++)
            {
                if (pProp->IsInVersion(iVer))
                {
                    XMLElement *pVersion = pDoc->NewElement("version");
                    pVersion->SetText(*pMaster->mGameVersions[iVer]);
                    pVersions->LinkEndChild(pVersion);
                }
            }
        }

        // Default
        if (pProp->CanHaveDefault() && pProp->Game() >= eEchoesDemo)
        {
            XMLElement *pDefault = pDoc->NewElement("default");
            pDefault->SetText(*pProp->DefaultToString());
            pElem->LinkEndChild(pDefault);
        }

        // Description
        if (!pProp->Description().IsEmpty())
        {
            XMLElement *pDesc = pDoc->NewElement("description");
            pDesc->SetText(*pProp->Description());
            pElem->LinkEndChild(pDesc);
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
            XMLElement *pCookPref = pDoc->NewElement("cook_pref");
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

            ExtensionsString = ExtensionsString.ChopBack(1); // Remove extra comma
            if (ExtensionsString.IsEmpty()) ExtensionsString = "UNKN";
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
                SaveEnumTemplate(pEnum);
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
                SaveBitfieldTemplate(pBitfield);
                pElem->SetAttribute("template", *pBitfield->mSourceFile);
            }
        }

        // Struct/array-specific parameters
        else if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
        {
            // Element Name
            if (pProp->Type() == eArrayProperty)
            {
                CArrayTemplate *pArray = static_cast<CArrayTemplate*>(pProp);

                if (!pArray->ElementName().IsEmpty())
                {
                    XMLElement *pElement = pDoc->NewElement("element_name");
                    pElement->SetText(*static_cast<CArrayTemplate*>(pProp)->ElementName());
                    pElem->LinkEndChild(pElement);
                }
            }

            // Sub-properties
            CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp);

            if (pStruct->mSourceFile.IsEmpty())
                SaveProperties(pDoc, pElem, pStruct);

            else
            {
                CStructTemplate *pOriginal = pMaster->StructAtSource(pStruct->mSourceFile);

                if (pOriginal)
                    SavePropertyOverrides(pDoc, pElem, pStruct, pOriginal);

                pElem->SetAttribute("template", *pStruct->mSourceFile);
            }
        }
    }
}

void CTemplateWriter::SavePropertyOverrides(XMLDocument *pDoc, XMLElement *pParent, CStructTemplate *pStruct, CStructTemplate *pOriginal)
{
    if (!pStruct->StructDataMatches(pOriginal))
    {
        // Create base element
        XMLElement *pPropsBlock = pDoc->NewElement("properties");
        pParent->LinkEndChild(pPropsBlock);

        for (u32 iProp = 0; iProp < pStruct->Count(); iProp++)
        {
            IPropertyTemplate *pProp = pStruct->PropertyByIndex(iProp);
            IPropertyTemplate *pSource = pOriginal->PropertyByIndex(iProp);

            if (!pProp->Matches(pSource))
            {
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

                pPropsBlock->LinkEndChild(pElem);

                // ID
                u32 ID = pProp->PropertyID();
                pElem->SetAttribute("ID", *TString::HexString(pProp->PropertyID(), (ID > 0xFF ? 8 : 2)));

                // Name
                if (pProp->Name() != pSource->Name())
                    pElem->SetAttribute("name", *pProp->Name());

                // Default
                if (pProp->CanHaveDefault() && !pProp->RawDefaultValue()->Matches(pSource->RawDefaultValue()))
                {
                    XMLElement *pDefault = pDoc->NewElement("default");
                    pDefault->SetText(*pProp->DefaultToString());
                    pElem->LinkEndChild(pDefault);
                }

                // Description
                if (pProp->Description() != pSource->Description())
                {
                    XMLElement *pDesc = pDoc->NewElement("description");
                    pDesc->SetText(*pProp->Description());
                    pElem->LinkEndChild(pDesc);
                }

                // Range
                if (pProp->IsNumerical())
                {
                    TString Range = pProp->RangeToString();

                    if (Range != pSource->RangeToString())
                    {
                        XMLElement *pRange = pDoc->NewElement("range");
                        pRange->SetText(*Range);
                        pElem->LinkEndChild(pRange);
                    }
                }

                // Suffix
                if (pProp->Suffix() != pSource->Suffix())
                {
                    XMLElement *pSuffix = pDoc->NewElement("suffix");
                    pSuffix->SetText(*pProp->Suffix());
                    pElem->LinkEndChild(pSuffix);
                }

                // Cook Pref
                if (pProp->CookPreference() != pSource->CookPreference())
                {
                    XMLElement *pCookPref = pDoc->NewElement("cook_pref");

                    TString PrefStr;
                    if (pProp->CookPreference() == eAlwaysCook) PrefStr = "always";
                    else if (pProp->CookPreference() == eNeverCook) PrefStr = "never";
                    else PrefStr = "none";

                    pCookPref->SetText(*PrefStr);
                    pElem->LinkEndChild(pCookPref);
                }

                // File-specific parameters
                if (pProp->Type() == eFileProperty)
                {
                    CFileTemplate *pFile = static_cast<CFileTemplate*>(pProp);
                    CFileTemplate *pSourceFile = static_cast<CFileTemplate*>(pSource);

                    if (pFile->Extensions() != pSourceFile->Extensions())
                    {
                        TString ExtensionsString;

                        for (auto it = pFile->Extensions().begin(); it != pFile->Extensions().end(); it++)
                            ExtensionsString += *it + ",";

                        ExtensionsString = ExtensionsString.ChopBack(1);
                        if (ExtensionsString.IsEmpty()) ExtensionsString = "UNKN";
                        pElem->SetAttribute("extensions", *ExtensionsString);
                    }
                }

                // Struct/array-specific parameters
                else if (pProp->Type() == eStructProperty || pProp->Type() == eArrayProperty)
                {
                    CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp);
                    CStructTemplate *pSourceStruct = static_cast<CStructTemplate*>(pSource);
                    SavePropertyOverrides(pDoc, pElem, pStruct, pSourceStruct);
                }
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
        pElem->SetAttribute("ID", *TString::HexString(EnumerID, (EnumerID > 0xFF ? 8 : 2)));
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
        pElem->SetAttribute("mask", *TString::HexString(pTemp->FlagMask(iFlag)));
        pElem->SetAttribute("name", *pTemp->FlagName(iFlag));
        pFlags->LinkEndChild(pElem);
    }
}
