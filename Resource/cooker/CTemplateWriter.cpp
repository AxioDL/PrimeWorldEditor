#include "CTemplateWriter.h"
#include "../cooker/CWorldCooker.h"
#include <tinyxml2.h>
#include <boost/filesystem.hpp>

using namespace tinyxml2;

CTemplateWriter::CTemplateWriter()
{
}

void CTemplateWriter::SaveAllTemplates()
{
    // Create directory
    std::list<CMasterTemplate*> masterList = CMasterTemplate::GetMasterList();
    std::string out = "../templates/";
    boost::filesystem::create_directory(out);

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
        pGameName->SetText(pMaster->mGameName.c_str());

        XMLElement *pWorldVersion = gameList.NewElement("mlvl");
        u32 versionNumber = CWorldCooker::GetMLVLVersion(pMaster->GetGame());
        pWorldVersion->SetText(StringUtil::ToHexString(versionNumber, true, true, 2).c_str());

        XMLElement *pTempPath = gameList.NewElement("master");
        pTempPath->SetText(pMaster->mSourceFile.c_str());

        pGame->LinkEndChild(pGameName);
        pGame->LinkEndChild(pWorldVersion);
        pGame->LinkEndChild(pTempPath);
        pBase->LinkEndChild(pGame);
    }

    gameList.SaveFile((out + "GameList.xml").c_str());
}

void CTemplateWriter::SaveGameTemplates(CMasterTemplate *pMaster, const std::string& dir)
{
    // Create directory
    std::string outFile = dir + pMaster->mSourceFile;
    std::string outDir = StringUtil::GetFileDirectory(outFile);
    boost::filesystem::create_directory(outDir);

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
        std::string objID;
        u32 intID = (it->second)->ObjectID();
        if (intID <= 0xFF) objID = StringUtil::ToHexString(intID, true, true, 2);
        else objID = CFourCC(intID).ToString();

        XMLElement *pObj = master.NewElement("object");
        pObj->SetAttribute("ID", objID.c_str());
        pObj->SetAttribute("template", (it->second)->mSourceFile.c_str());
        pObjects->LinkEndChild(pObj);
    }

    // Write script states/messages
    std::map<u32, std::string> *pMaps[2] = { &pMaster->mStates, &pMaster->mMessages };
    std::string types[2] = { "state", "message" };

    for (u32 iScr = 0; iScr < 2; iScr++)
    {
        XMLElement *pElem = master.NewElement((types[iScr] + "s").c_str());
        pBase->LinkEndChild(pElem);

        for (auto it = pMaps[iScr]->begin(); it != pMaps[iScr]->end(); it++)
        {
            std::string ID;
            if (it->first <= 0xFF) ID = StringUtil::ToHexString(it->first, true, true, 2);
            else ID = CFourCC(it->first).ToString();

            XMLElement *pSubElem = master.NewElement(types[iScr].c_str());
            pSubElem->SetAttribute("ID", ID.c_str());
            pSubElem->SetAttribute("name", (it->second).c_str());
            pElem->LinkEndChild(pSubElem);
        }
    }

    // Save file
    master.SaveFile(outFile.c_str());
}

void CTemplateWriter::SavePropertyList(CMasterTemplate *pMaster, const std::string& dir)
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
            pElem->SetAttribute("ID", StringUtil::ToHexString(pTemp->PropertyID(), true, true, 8).c_str());
            pElem->SetAttribute("name", pTemp->Name().c_str());

            if (!pStructTemp->mSourceFile.empty())
            {
                SaveStructTemplate(pStructTemp, pMaster, dir);
                pElem->SetAttribute("template", pStructTemp->mSourceFile.c_str());
            }

            pBase->LinkEndChild(pElem);
        }

        else
        {
            XMLElement *pElem = list.NewElement("property");
            pElem->SetAttribute("ID", StringUtil::ToHexString(pTemp->PropertyID(), true, true, 8).c_str());
            pElem->SetAttribute("name", pTemp->Name().c_str());
            pElem->SetAttribute("type", PropEnumToPropString(pTemp->Type()).c_str());

            if (pTemp->Type() == eFileProperty)
            {
                // Construct extension list string
                CFileTemplate *pFileProp = static_cast<CFileTemplate*>(pTemp);
                const CStringList& extensions = pFileProp->Extensions();

                std::string strList = "";

                for (auto it = extensions.begin(); it != extensions.end();)
                {
                    strList += *it;
                    it++;
                    if (it != extensions.end()) strList += ",";
                }

                pElem->SetAttribute("ext", strList.c_str());
            }

            pBase->LinkEndChild(pElem);
        }
    }

    list.SaveFile((dir + "Properties.xml").c_str());
}

void CTemplateWriter::SaveScriptTemplate(CScriptTemplate *pTemp, const std::string& dir)
{
    // Create directory
    std::string outFile = dir + pTemp->mSourceFile;
    std::string outDir = StringUtil::GetFileDirectory(outFile);
    boost::filesystem::create_directory(outDir);

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
    pName->SetText(pTemp->TemplateName().c_str());
    pBase->LinkEndChild(pName);

    // Write properties
    for (auto it = pTemp->mPropertySets.begin(); it != pTemp->mPropertySets.end(); it++)
    {
        XMLElement *pProperties = scriptXML.NewElement("properties");
        pProperties->SetAttribute("version", it->SetName.c_str());
        SaveProperties(&scriptXML, pProperties, it->pBaseStruct, pTemp->MasterTemplate(), dir);
        pBase->LinkEndChild(pProperties);
    }

    // Write editor properties
    XMLElement *pEditor = scriptXML.NewElement("editor");
    pBase->LinkEndChild(pEditor);

    // Editor Properties
    XMLElement *pEditorProperties = scriptXML.NewElement("properties");
    pEditor->LinkEndChild(pEditorProperties);

    std::string propNames[6] = {
        "InstanceName", "Position", "Rotation",
        "Scale", "Active", "LightParameters"
    };

    TIDString *pPropStrings[6] = {
        &pTemp->mNameIDString, &pTemp->mPositionIDString, &pTemp->mRotationIDString,
        &pTemp->mScaleIDString, &pTemp->mActiveIDString, &pTemp->mLightParametersIDString
    };

    for (u32 iProp = 0; iProp < 6; iProp++)
    {
        if (!pPropStrings[iProp]->empty())
        {
            XMLElement *pProperty = scriptXML.NewElement("property");
            pProperty->SetAttribute("name", propNames[iProp].c_str());
            pProperty->SetAttribute("ID", pPropStrings[iProp]->c_str());
            pEditorProperties->LinkEndChild(pProperty);
        }
    }

    // Editor Assets
    XMLElement *pAssets = scriptXML.NewElement("assets");
    pEditor->LinkEndChild(pAssets);

    for (auto it = pTemp->mAssets.begin(); it != pTemp->mAssets.end(); it++)
    {
        std::string source = (it->AssetSource == CScriptTemplate::SEditorAsset::eFile ? "file" : "property");
        std::string type;

        switch (it->AssetType)
        {
        case CScriptTemplate::SEditorAsset::eModel:      type = "model"; break;
        case CScriptTemplate::SEditorAsset::eAnimParams: type = "animparams"; break;
        case CScriptTemplate::SEditorAsset::eCollision:  type = "collision"; break;
        }

        s32 force = -1;
        if (it->AssetSource == CScriptTemplate::SEditorAsset::eAnimParams) force = it->ForceNodeIndex;

        XMLElement *pAsset = scriptXML.NewElement(type.c_str());
        pAsset->SetAttribute("source", source.c_str());
        if (force >= 0) pAsset->SetAttribute("force", std::to_string(force).c_str());
        pAsset->SetText(it->AssetLocation.c_str());
        pAssets->LinkEndChild(pAsset);
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
        auto GetVolumeString = [](EVolumeShape shape) -> std::string
        {
            switch (shape)
            {
            case eBoxShape:            return "Box";
            case eAxisAlignedBoxShape: return "AxisAlignedBox";
            case eEllipsoidShape:      return "Ellipsoid";
            case eCylinderShape:       return "Cylinder";
            case eCylinderLargeShape:  return "CylinderLarge";
            case eConditionalShape:    return "Conditional";
            default:                   return "INVALID";
            }
        };

        pVolume->SetAttribute("shape", GetVolumeString(pTemp->mVolumeShape).c_str());

        if (pTemp->mVolumeShape == eConditionalShape)
        {
            pVolume->SetAttribute("propertyID", pTemp->mVolumeConditionIDString.c_str());

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
                std::string strVal;

                if (pProp->Type() == eBoolProperty)
                    strVal = (it->Value == 1 ? "true" : "false");
                else
                    strVal = StringUtil::ToHexString((u32) it->Value, true, true, (it->Value > 0xFF ? 8 : 2));

                XMLElement *pCondition = scriptXML.NewElement("condition");
                pCondition->SetAttribute("value", strVal.c_str());
                pCondition->SetAttribute("shape", GetVolumeString(it->Shape).c_str());
                pVolume->LinkEndChild(pCondition);
            }
        }
    }

    // Write to file
    scriptXML.SaveFile(outFile.c_str());
}

void CTemplateWriter::SaveStructTemplate(CStructTemplate *pTemp, CMasterTemplate *pMaster, const std::string& dir)
{
    // Create directory
    std::string outFile = dir + pTemp->mSourceFile;
    std::string outDir = StringUtil::GetFileDirectory(outFile);
    std::string name = StringUtil::GetFileName(pTemp->mSourceFile);
    boost::filesystem::create_directory(outDir);

    // Create new document and write struct properties to it
    XMLDocument structXML;

    XMLDeclaration *pDecl = structXML.NewDeclaration();
    structXML.LinkEndChild(pDecl);

    XMLElement *pBase = structXML.NewElement("struct");
    pBase->SetAttribute("name", name.c_str());
    pBase->SetAttribute("type", (pTemp->IsSingleProperty() ? "single" : "multi"));
    SaveProperties(&structXML, pBase, pTemp, pMaster, dir);
    structXML.LinkEndChild(pBase);

    structXML.SaveFile(outFile.c_str());
}

void CTemplateWriter::SaveEnumTemplate(CEnumTemplate *pTemp, const std::string& dir)
{
    // Create directory
    std::string outFile = dir + pTemp->mSourceFile;
    std::string outDir = StringUtil::GetFileDirectory(outFile);
    std::string name = StringUtil::GetFileName(pTemp->mSourceFile);
    boost::filesystem::create_directory(outDir);

    // Create new document and write enumerators to it
    XMLDocument enumXML;

    XMLDeclaration *pDecl = enumXML.NewDeclaration();
    enumXML.LinkEndChild(pDecl);

    XMLElement *pBase = enumXML.NewElement("enum");
    SaveEnumerators(&enumXML, pBase, pTemp);
    enumXML.LinkEndChild(pBase);

    enumXML.SaveFile(outFile.c_str());
}

void CTemplateWriter::SaveProperties(XMLDocument *pDoc, XMLElement *pParent, CStructTemplate *pTemp, CMasterTemplate *pMaster, const std::string& dir)
{
    for (u32 iProp = 0; iProp < pTemp->Count(); iProp++)
    {
        CPropertyTemplate *pProp = pTemp->PropertyByIndex(iProp);
        u32 propID = (pProp->PropertyID() == 0xFFFFFFFF ? iProp : pProp->PropertyID());
        std::string strID = StringUtil::ToHexString(propID, true, true, (propID > 0xFF ? 8 : 2));

        if (pProp->Type() == eStructProperty)
        {
            CStructTemplate *pStructTemp = static_cast<CStructTemplate*>(pProp);
            bool isExternal = (!pStructTemp->mSourceFile.empty());

            XMLElement *pElem = pDoc->NewElement("struct");
            pElem->SetAttribute("ID", strID.c_str());

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1) || pTemp->IsSingleProperty())
            {
                pElem->SetAttribute("name", pProp->Name().c_str());
            }

            if (!isExternal) {
                std::string type = pStructTemp->IsSingleProperty() ? "single" : "multi";
                pElem->SetAttribute("type", type.c_str());
            }

            // Only save properties if this is a multi struct, or if there is no master property list
            if (!pMaster->HasPropertyList() || !pStructTemp->IsSingleProperty())
            {
                // Embed struct or save to external XML?
                if (!pStructTemp->mSourceFile.empty())
                {
                    SaveStructTemplate(pStructTemp, pMaster, dir);
                    pElem->SetAttribute("template", pStructTemp->mSourceFile.c_str());
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
            bool isExternal = (!pEnumTemp->mSourceFile.empty());

            XMLElement *pElem = pDoc->NewElement("enum");
            pElem->SetAttribute("ID", strID.c_str());

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1))
                pElem->SetAttribute("name", pProp->Name().c_str());

            if (isExternal)
            {
                SaveEnumTemplate(pEnumTemp, dir);
                pElem->SetAttribute("template", pEnumTemp->mSourceFile.c_str());
            }

            else
            {
                SaveEnumerators(pDoc, pElem, pEnumTemp);
            }

            pParent->LinkEndChild(pElem);
        }
        else
        {
            XMLElement *pElem = pDoc->NewElement("property");
            pElem->SetAttribute("ID", strID.c_str());

            if ((!pMaster->HasPropertyList()) || (pProp->PropertyID() == -1) || pTemp->IsSingleProperty())
            {
                pElem->SetAttribute("name", pProp->Name().c_str());
                pElem->SetAttribute("type", PropEnumToPropString(pProp->Type()).c_str());

                if (pProp->Type() == eFileProperty)
                {
                    // Construct extension list string
                    CFileTemplate *pFileProp = static_cast<CFileTemplate*>(pProp);
                    const CStringList& extensions = pFileProp->Extensions();

                    std::string strList = "";

                    for (auto it = extensions.begin(); it != extensions.end();)
                    {
                        strList += *it;
                        it++;
                        if (it != extensions.end()) strList += ",";
                    }

                    pElem->SetAttribute("ext", strList.c_str());
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
        pElem->SetAttribute("ID", StringUtil::ToHexString(pTemp->EnumeratorID(iEnum), true, true, 8).c_str());
        pElem->SetAttribute("name", pTemp->EnumeratorName(iEnum).c_str());
        pParent->LinkEndChild(pElem);
    }
}
