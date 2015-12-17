#include "CTemplateWriter.h"
#include "CWorldCooker.h"

#include <boost/filesystem.hpp>
#include <tinyxml2.h>

using namespace tinyxml2;

CTemplateWriter::CTemplateWriter()
{
}

void CTemplateWriter::SaveAllTemplates()
{
    // Create directory
    std::list<CMasterTemplate*> masterList = CMasterTemplate::GetMasterList();
    TString out = "../templates/";
    boost::filesystem::create_directory(out.ToStdString());

    // Resave master templates
    for (auto it = masterList.begin(); it != masterList.end(); it++)
        SaveGameTemplates(*it, out);

    // Resave game list
    XMLDocument gameList;

    XMLDeclaration *pDecl = gameList.NewDeclaration();
    gameList.LinkEndChild(pDecl);

    XMLElement *pBase = gameList.NewElement("GameList");
    pBase->SetAttribute("version", 3);
    gameList.LinkEndChild(pBase);

    for (auto it = masterList.begin(); it != masterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        XMLElement *pGame = gameList.NewElement("game");

        XMLElement *pGameName = gameList.NewElement("name");
        pGameName->SetText(*pMaster->mGameName);

        XMLElement *pWorldVersion = gameList.NewElement("mlvl");
        u32 versionNumber = CWorldCooker::GetMLVLVersion(pMaster->GetGame());
        pWorldVersion->SetText(*TString::HexString(versionNumber, true, true, 2));

        XMLElement *pTempPath = gameList.NewElement("master");
        pTempPath->SetText(*pMaster->mSourceFile);

        pGame->LinkEndChild(pGameName);
        pGame->LinkEndChild(pWorldVersion);
        pGame->LinkEndChild(pTempPath);
        pBase->LinkEndChild(pGame);
    }

    gameList.SaveFile(*(out + "GameList.xml"));
}

void CTemplateWriter::SaveGameTemplates(CMasterTemplate *pMaster, const TString& dir)
{
    // Create directory
    TString outFile = dir + pMaster->mSourceFile;
    TString outDir = outFile.GetFileDirectory();
    boost::filesystem::create_directory(outDir.ToStdString());

    // Resave script templates
    for (auto it = pMaster->mTemplates.begin(); it != pMaster->mTemplates.end(); it++)
        SaveScriptTemplate(it->second, outDir);

    // Resave master template
    XMLDocument master;

    XMLDeclaration *pDecl = master.NewDeclaration();
    master.LinkEndChild(pDecl);

    XMLElement *pBase = master.NewElement("MasterTemplate");
    pBase->SetAttribute("version", 3);
    master.LinkEndChild(pBase);

    // Write property list
    if (!pMaster->mPropertyList.empty())
    {
        SavePropertyList(pMaster, outDir);

        XMLElement *pPropList = master.NewElement("properties");
        pPropList->SetText("Properties.xml");
        pBase->LinkEndChild(pPropList);
    }

    // Write script objects
    XMLElement *pObjects = master.NewElement("objects");
    pBase->LinkEndChild(pObjects);

    for (auto it = pMaster->mTemplates.begin(); it != pMaster->mTemplates.end(); it++)
    {
        TString objID;
        u32 intID = (it->second)->ObjectID();
        if (intID <= 0xFF) objID = TString::HexString(intID, true, true, 2);
        else objID = CFourCC(intID).ToString();

        XMLElement *pObj = master.NewElement("object");
        pObj->SetAttribute("ID", *objID);
        pObj->SetAttribute("template", *(it->second)->mSourceFile);
        pObjects->LinkEndChild(pObj);
    }

    // Write script states/messages
    std::map<u32, TString> *pMaps[2] = { &pMaster->mStates, &pMaster->mMessages };
    TString types[2] = { "state", "message" };

    for (u32 iScr = 0; iScr < 2; iScr++)
    {
        XMLElement *pElem = master.NewElement(*(types[iScr] + "s"));
        pBase->LinkEndChild(pElem);

        for (auto it = pMaps[iScr]->begin(); it != pMaps[iScr]->end(); it++)
        {
            TString ID;
            if (it->first <= 0xFF) ID = TString::HexString(it->first, true, true, 2);
            else ID = CFourCC(it->first).ToString();

            XMLElement *pSubElem = master.NewElement(*types[iScr]);
            pSubElem->SetAttribute("ID", *ID);
            pSubElem->SetAttribute("name", *(it->second));
            pElem->LinkEndChild(pSubElem);
        }
    }

    // Save file
    master.SaveFile(*outFile);
}

void CTemplateWriter::SavePropertyList(CMasterTemplate *pMaster, const TString& dir)
{
    // Create XML
    XMLDocument list;

    XMLDeclaration *pDecl = list.NewDeclaration();
    list.LinkEndChild(pDecl);

    XMLElement *pBase = list.NewElement("Properties");
    pBase->SetAttribute("version", 3);
    list.LinkEndChild(pBase);

    // Write properties
    for (auto it = pMaster->mPropertyList.begin(); it != pMaster->mPropertyList.end(); it++)
    {
        CPropertyTemplate *pTemp = it->second;

        if (pTemp->Type() == eStructProperty)
        {
            CStructTemplate *pStructTemp = static_cast<CStructTemplate*>(pTemp);

            XMLElement *pElem = list.NewElement("struct");
            pElem->SetAttribute("ID", *TString::HexString(pTemp->PropertyID(), true, true, 8));
            pElem->SetAttribute("name", *pTemp->Name());

            if (!pStructTemp->mSourceFile.IsEmpty())
            {
                SaveStructTemplate(pStructTemp, pMaster, dir);
                pElem->SetAttribute("template", *pStructTemp->mSourceFile);
            }

            pBase->LinkEndChild(pElem);
        }

        else
        {
            XMLElement *pElem = list.NewElement("property");
            pElem->SetAttribute("ID", *TString::HexString(pTemp->PropertyID(), true, true, 8));
            pElem->SetAttribute("name", *pTemp->Name());
            pElem->SetAttribute("type", *PropEnumToPropString(pTemp->Type()));

            if (pTemp->Type() == eFileProperty)
            {
                // Construct extension list string
                CFileTemplate *pFileProp = static_cast<CFileTemplate*>(pTemp);
                const TStringList& extensions = pFileProp->Extensions();

                TString strList = "";

                for (auto it = extensions.begin(); it != extensions.end();)
                {
                    strList += *it;
                    it++;
                    if (it != extensions.end()) strList += ",";
                }

                pElem->SetAttribute("ext", *strList);
            }

            pBase->LinkEndChild(pElem);
        }
    }

    list.SaveFile(*(dir + "Properties.xml"));
}

void CTemplateWriter::SaveScriptTemplate(CScriptTemplate *pTemp, const TString& dir)
{
    // Create directory
    TString outFile = dir + pTemp->mSourceFile;
    TString outDir = outFile.GetFileDirectory();
    boost::filesystem::create_directory(*outDir);

    // Create new document
    XMLDocument scriptXML;

    XMLDeclaration *pDecl = scriptXML.NewDeclaration();
    scriptXML.LinkEndChild(pDecl);

    // Base element
    XMLElement *pBase = scriptXML.NewElement("ScriptTemplate");
    pBase->SetAttribute("version", 3.0f);
    scriptXML.LinkEndChild(pBase);

    // Write object name
    XMLElement *pName = scriptXML.NewElement("name");
    pName->SetText(*pTemp->TemplateName());
    pBase->LinkEndChild(pName);

    // Write properties
    for (auto it = pTemp->mPropertySets.begin(); it != pTemp->mPropertySets.end(); it++)
    {
        XMLElement *pProperties = scriptXML.NewElement("properties");
        pProperties->SetAttribute("version", *it->SetName);
        SaveProperties(&scriptXML, pProperties, it->pBaseStruct, pTemp->MasterTemplate(), dir);
        pBase->LinkEndChild(pProperties);
    }

    // Write editor properties
    XMLElement *pEditor = scriptXML.NewElement("editor");
    pBase->LinkEndChild(pEditor);

    // Editor Properties
    XMLElement *pEditorProperties = scriptXML.NewElement("properties");
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
            XMLElement *pProperty = scriptXML.NewElement("property");
            pProperty->SetAttribute("name", *propNames[iProp]);
            pProperty->SetAttribute("ID", **pPropStrings[iProp]);
            pEditorProperties->LinkEndChild(pProperty);
        }
    }

    // Editor Assets
    XMLElement *pAssets = scriptXML.NewElement("assets");
    pEditor->LinkEndChild(pAssets);

    for (auto it = pTemp->mAssets.begin(); it != pTemp->mAssets.end(); it++)
    {
        TString source = (it->AssetSource == CScriptTemplate::SEditorAsset::eFile ? "file" : "property");
        TString type;

        switch (it->AssetType)
        {
        case CScriptTemplate::SEditorAsset::eModel:      type = "model"; break;
        case CScriptTemplate::SEditorAsset::eAnimParams: type = "animparams"; break;
        case CScriptTemplate::SEditorAsset::eBillboard:  type = "billboard"; break;
        case CScriptTemplate::SEditorAsset::eCollision:  type = "collision"; break;
        }

        s32 force = -1;
        if (it->AssetSource == CScriptTemplate::SEditorAsset::eAnimParams) force = it->ForceNodeIndex;

        XMLElement *pAsset = scriptXML.NewElement(*type);
        pAsset->SetAttribute("source", *source);
        if (force >= 0) pAsset->SetAttribute("force", std::to_string(force).c_str());
        pAsset->SetText(*it->AssetLocation);
        pAssets->LinkEndChild(pAsset);
    }

    // Preview Scale
    if (pTemp->mPreviewScale != 1.f)
    {
        XMLElement *pPreviewScale = scriptXML.NewElement("preview_scale");
        pEditor->LinkEndChild(pPreviewScale);
        pPreviewScale->SetText(pTemp->mPreviewScale);
    }

    // Rot/Scale Type
    XMLElement *pRotType = scriptXML.NewElement("rotation_type");
    pEditor->LinkEndChild(pRotType);
    pRotType->SetText(pTemp->mRotationType == CScriptTemplate::eRotationEnabled ? "enabled" : "disabled");

    XMLElement *pScaleType = scriptXML.NewElement("scale_type");
    pEditor->LinkEndChild(pScaleType);

    if (pTemp->mScaleType != CScriptTemplate::eScaleVolume)
        pScaleType->SetText(pTemp->mScaleType == CScriptTemplate::eScaleEnabled ? "enabled" : "disabled");

    else
    {
        pScaleType->SetText("volume");

        // Volume Preview
        XMLElement *pVolume = scriptXML.NewElement("preview_volume");
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
            CPropertyTemplate *pProp;

            for (auto it = pTemp->mPropertySets.begin(); it != pTemp->mPropertySets.end(); it++)
            {
                pProp = it->pBaseStruct->PropertyByIDString(pTemp->mVolumeConditionIDString);
                if (pProp) break;
            }

            // Write conditions
            for (auto it = pTemp->mVolumeConditions.begin(); it != pTemp->mVolumeConditions.end(); it++)
            {
                // Value should be an integer, or a boolean condition?
                TString strVal;

                if (pProp->Type() == eBoolProperty)
                    strVal = (it->Value == 1 ? "true" : "false");
                else
                    strVal = TString::HexString((u32) it->Value, true, true, (it->Value > 0xFF ? 8 : 2));

                XMLElement *pCondition = scriptXML.NewElement("condition");
                pCondition->SetAttribute("value", *strVal);
                pCondition->SetAttribute("shape", *GetVolumeString(it->Shape));
                if (it->Scale != 1.f) pCondition->SetAttribute("scale", it->Scale);
                pVolume->LinkEndChild(pCondition);
            }
        }
    }

    // Write to file
    scriptXML.SaveFile(*outFile);
}

void CTemplateWriter::SaveStructTemplate(CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& dir)
{
    // Create directory
    TString outFile = dir + pTemp->mSourceFile;
    TString outDir = outFile.GetFileDirectory();
    TString name = pTemp->mSourceFile.GetFileName();
    boost::filesystem::create_directory(outDir.ToStdString());

    // Create new document and write struct properties to it
    XMLDocument structXML;

    XMLDeclaration *pDecl = structXML.NewDeclaration();
    structXML.LinkEndChild(pDecl);

    XMLElement *pBase = structXML.NewElement("struct");
    pBase->SetAttribute("name", *name);
    pBase->SetAttribute("type", (pTemp->IsSingleProperty() ? "single" : "multi"));
    SaveProperties(&structXML, pBase, pTemp, pMaster, dir);
    structXML.LinkEndChild(pBase);

    structXML.SaveFile(*outFile);
}

void CTemplateWriter::SaveEnumTemplate(CEnumTemplate *pTemp, const TString& dir)
{
    // Create directory
    TString outFile = dir + pTemp->mSourceFile;
    TString outDir = outFile.GetFileDirectory();
    TString name = pTemp->mSourceFile.GetFileName(false);
    boost::filesystem::create_directory(*outDir);

    // Create new document and write enumerators to it
    XMLDocument enumXML;

    XMLDeclaration *pDecl = enumXML.NewDeclaration();
    enumXML.LinkEndChild(pDecl);

    XMLElement *pBase = enumXML.NewElement("enum");
    pBase->SetAttribute("name", *name);
    SaveEnumerators(&enumXML, pBase, pTemp);
    enumXML.LinkEndChild(pBase);

    enumXML.SaveFile(*outFile);
}

void CTemplateWriter::SaveBitfieldTemplate(CBitfieldTemplate *pTemp, const TString& dir)
{
    // Create directory
    TString outFile = dir + pTemp->mSourceFile;
    TString outDir = outFile.GetFileDirectory();
    TString name = pTemp->mSourceFile.GetFileName();
    boost::filesystem::create_directory(*outDir);

    // Create new document and write enumerators to it
    XMLDocument bitfieldXML;

    XMLDeclaration *pDecl = bitfieldXML.NewDeclaration();
    bitfieldXML.LinkEndChild(pDecl);

    XMLElement *pBase = bitfieldXML.NewElement("bitfield");
    pBase->SetAttribute("name", *name);
    SaveBitFlags(&bitfieldXML, pBase, pTemp);
    bitfieldXML.LinkEndChild(pBase);

    bitfieldXML.SaveFile(*outFile);
}

void CTemplateWriter::SaveProperties(XMLDocument *pDoc, XMLElement *pParent, CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& dir)
{
    for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
    {
        CPropertyTemplate *pProp = pTemp->PropertyByIndex(iProp);
        u32 propID = (pProp->PropertyID() == 0xFFFFFFFF ? iProp : pProp->PropertyID());
        TString strID = TString::HexString(propID, true, true, (propID > 0xFF ? 8 : 2));

        if (pProp->Type() == eStructProperty)
        {
            CStructTemplate *pStructTemp = static_cast<CStructTemplate*>(pProp);
            bool isExternal = (!pStructTemp->mSourceFile.IsEmpty());

            XMLElement *pElem = pDoc->NewElement("struct");
            pElem->SetAttribute("ID", *strID);

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1) || pTemp->IsSingleProperty())
            {
                pElem->SetAttribute("name", *pProp->Name());
            }

            if (!isExternal) {
                TString type = pStructTemp->IsSingleProperty() ? "single" : "multi";
                pElem->SetAttribute("type", *type);
            }

            // Only save properties if this is a multi struct, or if there is no master property list
            if (!pMaster->HasPropertyList() || !pStructTemp->IsSingleProperty())
            {
                // Embed struct or save to external XML?
                if (!pStructTemp->mSourceFile.IsEmpty())
                {
                    SaveStructTemplate(pStructTemp, pMaster, dir);
                    pElem->SetAttribute("template", *pStructTemp->mSourceFile);
                }

                else
                {
                    SaveProperties(pDoc, pElem, pStructTemp, pMaster, dir);
                }
            }

            pParent->LinkEndChild(pElem);
        }

        else if (pProp->Type() == eEnumProperty)
        {
            CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pProp);
            bool isExternal = (!pEnumTemp->mSourceFile.IsEmpty());

            XMLElement *pElem = pDoc->NewElement("enum");
            pElem->SetAttribute("ID", *strID);

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1))
                pElem->SetAttribute("name", *pProp->Name());

            if (isExternal)
            {
                SaveEnumTemplate(pEnumTemp, dir);
                pElem->SetAttribute("template", *pEnumTemp->mSourceFile);
            }

            else
            {
                SaveEnumerators(pDoc, pElem, pEnumTemp);
            }

            pParent->LinkEndChild(pElem);
        }
        else if (pProp->Type() == eBitfieldProperty)
        {
            CBitfieldTemplate *pBitfieldTemp = static_cast<CBitfieldTemplate*>(pProp);
            bool isExternal = (!pBitfieldTemp->mSourceFile.IsEmpty());

            XMLElement *pElem = pDoc->NewElement("bitfield");
            pElem->SetAttribute("ID", *strID);

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1))
                pElem->SetAttribute("name", *pProp->Name());

            if (isExternal)
            {
                SaveBitfieldTemplate(pBitfieldTemp, dir);
                pElem->SetAttribute("template", *pBitfieldTemp->mSourceFile);
            }

            else
            {
                SaveBitFlags(pDoc, pElem, pBitfieldTemp);
            }

            pParent->LinkEndChild(pElem);
        }
        else
        {
            XMLElement *pElem = pDoc->NewElement("property");
            pElem->SetAttribute("ID", *strID);

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1) || pTemp->IsSingleProperty())
            {
                pElem->SetAttribute("name", *pProp->Name());
                pElem->SetAttribute("type", *PropEnumToPropString(pProp->Type()));

                if (pProp->Type() == eFileProperty)
                {
                    // Construct extension list string
                    CFileTemplate *pFileProp = static_cast<CFileTemplate*>(pProp);
                    const TStringList& extensions = pFileProp->Extensions();

                    TString strList = "";

                    for (auto it = extensions.begin(); it != extensions.end();)
                    {
                        strList += *it;
                        it++;
                        if (it != extensions.end()) strList += ",";
                    }

                    pElem->SetAttribute("ext", *strList);
                }
            }

            pParent->LinkEndChild(pElem);
        }
    }
}

void CTemplateWriter::SaveEnumerators(XMLDocument *pDoc, XMLElement *pParent, CEnumTemplate *pTemp)
{
    for (u32 iEnum = 0; iEnum < pTemp->NumEnumerators(); iEnum++)
    {
        XMLElement *pElem = pDoc->NewElement("enumerator");
        pElem->SetAttribute("ID", *TString::HexString(pTemp->EnumeratorID(iEnum), true, true, 8));
        pElem->SetAttribute("name", *pTemp->EnumeratorName(iEnum));
        pParent->LinkEndChild(pElem);
    }
}

void CTemplateWriter::SaveBitFlags(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CBitfieldTemplate *pTemp)
{
    for (u32 iFlag = 0; iFlag < pTemp->NumFlags(); iFlag++)
    {
        XMLElement *pElem = pDoc->NewElement("bitflag");
        pElem->SetAttribute("mask", *TString::HexString(pTemp->FlagMask(iFlag), true, true, 8));
        pElem->SetAttribute("name", *pTemp->FlagName(iFlag));
        pParent->LinkEndChild(pElem);
    }
}
