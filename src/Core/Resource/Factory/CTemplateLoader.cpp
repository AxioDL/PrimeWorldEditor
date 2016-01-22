#include "CTemplateLoader.h"
#include "CAreaLoader.h"
#include "Core/Resource/Script/IPropertyTemplate.h"
#include "Core/Log.h"
#include <boost/filesystem.hpp>

const TString CTemplateLoader::mskTemplatesDir = "../templates/";
const TString CTemplateLoader::mskGameListPath = CTemplateLoader::mskTemplatesDir + "GameList.xml";

using namespace tinyxml2;

IPropertyTemplate* CTemplateLoader::LoadProperty(XMLElement *pElem, CStructTemplate *pStruct, const TString& rkTemplateName)
{
    TString NodeType = TString(pElem->Name()).ToLower();
    TString IDAttr = TString(pElem->Attribute("ID")).ToLower();
    TString TypeAttr = TString(pElem->Attribute("type")).ToLower();
    TString NameAttr = pElem->Attribute("name");
    TString TemplateAttr = pElem->Attribute("template");

    // Get ID + name
    if (IDAttr.IsEmpty())
    {
        Log::Error("Error reading " + rkTemplateName + "; ran into a property with no ID");
        return nullptr;
    }

    u32 ID = IDAttr.ToInt32();
    TString Name;

    if (!NameAttr.IsEmpty())
        Name = NameAttr;
    else if (mpMaster->HasPropertyList())
        Name = mpMaster->PropertyName(ID);
    else
    {
        Log::Error("Error reading " + rkTemplateName + " property " + TString::HexString(ID, true, true, 8) + "; this property doesn't have a name either in the template itself nor in the master list");
        return nullptr;
    }

    // Does the property already exist (eg is this an override)?
    IPropertyTemplate *pProp = pStruct->PropertyByID(ID);
    EPropertyType Type;
    bool IsNewProperty = false;

    // If it doesn't, then we'll need to create it.
    if (!pProp)
    {
        // Determine type
        TString TypeStr = (NodeType == "property" ? TypeAttr : NodeType);
        Type = PropStringToPropEnum(TypeStr);
        IsNewProperty = true;

        if (Type == eInvalidProperty)
        {
            if (TypeStr.IsEmpty())
                Log::Error("Error reading " + rkTemplateName + " property " + TString::HexString(ID, true, true, 8) + "; this property doesn't have a valid type set");
            else
                Log::Error("Error reading " + rkTemplateName + " property " + TString::HexString(ID, true, true, 8) + "; this property has an invalid type set: " + TypeStr);

            return nullptr;
        }

        pProp = CreateProperty(ID, Type, Name, pStruct);

        if (!pProp)
        {
            Log::Error("Error reading " + rkTemplateName + " property " + TString::HexString(ID, true, true, 8) + "; seem to have attempted to load a valid but unsupported property type? (" + TypeStr + ")");
            return nullptr;
        }
    }
    else
        Type = pProp->Type();

    // Common parameters
    XMLElement *pParams = pElem->FirstChildElement();

    while (pParams)
    {
        TString ParamName = TString(pParams->Name()).ToLower();
        TString ParamVal = TString(pParams->GetText());

        // Load versions
        if (ParamName == "versions")
        {
            XMLElement *pVersion = pParams->FirstChildElement("version");

            while (pVersion)
            {
                TString VerName = pVersion->GetText();
                u32 VerIdx = mpMaster->GetGameVersion(VerName);

                if (VerIdx == -1)
                    Log::Error("Error reading " + rkTemplateName + " property " + TString::HexString(ID, true, true, 8) + "; invalid version \"" + VerName + "\"");
                else
                    pProp->mAllowedVersions.push_back(VerIdx);

                pVersion = pVersion->NextSiblingElement("version");
            }
        }

        // Otherwise, delegate it to the template to parse the parameter.
        // (This is done because there's no common base class for typed properties, so it's tough to handle this in the template loader.)
        else pProp->SetParam(ParamName, ParamVal);

        pParams = pParams->NextSiblingElement();
    }

    // File-specific parameters
    if (Type == eFileProperty)
    {
        CFileTemplate *pFile = static_cast<CFileTemplate*>(pProp);
        TString ExtensionsAttr = pElem->Attribute("extensions");
        TStringList ExtensionsList = ExtensionsAttr.Split(", ");
        pFile->SetAllowedExtensions(ExtensionsList);
    }

    // Enum-specific parameters
    else if (Type == eEnumProperty)
    {
        CEnumTemplate *pEnum = static_cast<CEnumTemplate*>(pProp);

        // Load template
        if (!TemplateAttr.IsEmpty())
            LoadEnumTemplate(TemplateAttr, pEnum);

        // Load embedded enumerators
        XMLElement *pEnumerators = pElem->FirstChildElement("enumerators");

        if (pEnumerators)
            LoadEnumerators(pEnumerators, pEnum, rkTemplateName);
    }

    // Bitfield-specific parameters
    else if (Type == eBitfieldProperty)
    {
        CBitfieldTemplate *pBitfield = static_cast<CBitfieldTemplate*>(pProp);

        // Load template
        if (!TemplateAttr.IsEmpty())
            LoadBitfieldTemplate(TemplateAttr, pBitfield);

        // Load embedded flags
        XMLElement *pFlags = pElem->FirstChildElement("flags");

        if (pFlags)
            LoadBitFlags(pFlags, pBitfield, rkTemplateName);
    }

    // Struct-specific parameters
    else if ( (Type == eStructProperty) || (Type == eArrayProperty) )
    {
        CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp);

        // Load template or struct type
        if (!TemplateAttr.IsEmpty())
            LoadStructTemplate(TemplateAttr, pStruct);

        if (IsNewProperty && TemplateAttr.IsEmpty() && Type == eStructProperty)
            pStruct->mIsSingleProperty = (TypeAttr == "single" ? true : false);

        // Load sub-properties
        XMLElement *pProperties = pElem->FirstChildElement("properties");

        if (pProperties)
            LoadProperties(pProperties, pStruct, rkTemplateName);
    }

    return pProp;
}

#define CREATE_PROP_TEMP(Class) new Class(ID, rkName, eNoCookPreference, pStruct)
IPropertyTemplate* CTemplateLoader::CreateProperty(u32 ID, EPropertyType Type, const TString& rkName, CStructTemplate *pStruct)
{
    IPropertyTemplate *pOut = pStruct->PropertyByID(ID);

    switch (Type)
    {
    case eBoolProperty:      pOut = CREATE_PROP_TEMP(TBoolTemplate);      break;
    case eByteProperty:      pOut = CREATE_PROP_TEMP(TByteTemplate);      break;
    case eShortProperty:     pOut = CREATE_PROP_TEMP(TShortTemplate);     break;
    case eLongProperty:      pOut = CREATE_PROP_TEMP(TLongTemplate);      break;
    case eFloatProperty:     pOut = CREATE_PROP_TEMP(TFloatTemplate);     break;
    case eStringProperty:    pOut = CREATE_PROP_TEMP(TStringTemplate);    break;
    case eVector3Property:   pOut = CREATE_PROP_TEMP(TVector3Template);   break;
    case eColorProperty:     pOut = CREATE_PROP_TEMP(TColorTemplate);     break;
    case eFileProperty:      pOut = CREATE_PROP_TEMP(CFileTemplate);      break;
    case eCharacterProperty: pOut = CREATE_PROP_TEMP(CCharacterTemplate); break;
    case eEnumProperty:      pOut = CREATE_PROP_TEMP(CEnumTemplate);      break;
    case eBitfieldProperty:  pOut = CREATE_PROP_TEMP(CBitfieldTemplate);  break;
    case eArrayProperty:     pOut = CREATE_PROP_TEMP(CArrayTemplate);     break;
    case eStructProperty:    pOut = CREATE_PROP_TEMP(CStructTemplate);    break;
    }

    if (pOut)
        pStruct->mSubProperties.push_back(pOut);

    return pOut;
}

void CTemplateLoader::LoadStructTemplate(const TString& rkTemplateFileName, CStructTemplate *pStruct)
{
    XMLDocument Doc;
    OpenXML(mMasterDir + rkTemplateFileName, Doc);

    if (!Doc.Error())
    {
        XMLElement *pRootElem;

        if (pStruct->Type() == eStructProperty)
        {
            pRootElem = Doc.FirstChildElement("struct");

            if (!pRootElem)
            {
                Log::Error("Error reading struct template " + rkTemplateFileName + ": there is no root \"struct\" element");
                return;
            }

            TString TypeAttr = TString(pRootElem->Attribute("type")).ToLower();

            if (TypeAttr.IsEmpty())
            {
                Log::Error("Error reading struct template " + rkTemplateFileName + "; there is no struct type specified");
                return;
            }

            pStruct->mIsSingleProperty = (TypeAttr == "single" ? true : false);
        }

        else if (pStruct->Type() == eArrayProperty)
        {
            pRootElem = Doc.FirstChildElement("array");

            if (!pRootElem)
            {
                Log::Error("Error reading array template " + rkTemplateFileName + "; there is no root \"array\" element");
                return;
            }
        }

        // Read sub-properties
        XMLElement *pSubPropsElem = pRootElem->FirstChildElement("properties");

        if (pSubPropsElem)
            LoadProperties(pSubPropsElem, pStruct, rkTemplateFileName);

        else
            Log::Error("Error reading " + TString(pStruct->Type() == eStructProperty ? "struct" : "array") + " template " + rkTemplateFileName + "; there's no \"properties\" block element");
    }
}

void CTemplateLoader::LoadEnumTemplate(const TString& rkTemplateFileName, CEnumTemplate *pEnum)
{
    XMLDocument Doc;
    OpenXML(mMasterDir + rkTemplateFileName, Doc);

    if (!Doc.Error())
    {
        XMLElement *pRootElem = Doc.FirstChildElement("enum");

        if (!pRootElem)
        {
            Log::Error("Error reading enum template " + rkTemplateFileName + "; there is no root \"enum\" element");
            return;
        }

        XMLElement *pEnumers = pRootElem->FirstChildElement("enumerators");

        if (pEnumers)
            LoadEnumerators(pEnumers, pEnum, rkTemplateFileName);

        else
            Log::Error("Error reading enum template " + rkTemplateFileName + "; there is no \"enumerators\" block element");
    }
}

void CTemplateLoader::LoadBitfieldTemplate(const TString& rkTemplateFileName, CBitfieldTemplate *pBitfield)
{
    XMLDocument Doc;
    OpenXML(mMasterDir + rkTemplateFileName, Doc);

    if (!Doc.Error())
    {
        XMLElement *pRootElem = Doc.FirstChildElement("bitfield");

        if (!pRootElem)
        {
            Log::Error("Error reading bitfield template " + rkTemplateFileName + "; there is no root \"bitfield\" element");
            return;
        }

        XMLElement *pFlags = pRootElem->FirstChildElement("flags");

        if (pFlags)
            LoadBitFlags(pFlags, pBitfield, rkTemplateFileName);

        else
            Log::Error("Error reading bitfield template " + rkTemplateFileName + "; there is no \"flags\" block element");
    }
}

void CTemplateLoader::LoadProperties(XMLElement *pPropertiesElem, CStructTemplate *pStruct, const TString& rkTemplateName)
{
    XMLElement *pChild = pPropertiesElem->FirstChildElement();

    while (pChild)
    {
        TString NodeType = TString(pChild->Name()).ToLower();

        if ( (NodeType != "property") && (NodeType != "struct") && (NodeType != "enum") && (NodeType != "bitfield") && (NodeType != "array") )
        {
            Log::Error("Error reading " + rkTemplateName + "; a node in a properties block has an invalid name: " + NodeType);
        }

        // LoadProperty adds newly created properties to the struct, so we don't need to do anything other than call it for each sub-element.
        else
        {
            LoadProperty(pChild, pStruct, rkTemplateName);
        }

        pChild = pChild->NextSiblingElement();
    }

    pStruct->mVersionPropertyCounts.resize(mpMaster->NumGameVersions());
    pStruct->DetermineVersionPropertyCounts();
}

void CTemplateLoader::LoadEnumerators(XMLElement *pEnumeratorsElem, CEnumTemplate *pTemp, const TString& rkTemplateName)
{
    XMLElement *pChild = pEnumeratorsElem->FirstChildElement("enumerator");

    while (pChild)
    {
        const char *pkID = pChild->Attribute("ID");
        const char *pkName = pChild->Attribute("name");

        if (pkID && pkName)
            pTemp->mEnumerators.push_back(CEnumTemplate::SEnumerator(pkName, TString(pkID).ToInt32()));

        else
        {
            TString LogErrorBase = "Couldn't parse enumerator in " + rkTemplateName + "; ";

            if      (!pkID && pkName) Log::Error(LogErrorBase + "no valid ID (" + pkName + ")");
            else if (pkID && !pkName) Log::Error(LogErrorBase + "no valid name (ID " + pkID + ")");
            else Log::Error(LogErrorBase + "no valid ID or name");
        }

        pChild = pChild->NextSiblingElement("enumerator");
    }
}

void CTemplateLoader::LoadBitFlags(XMLElement *pFlagsElem, CBitfieldTemplate *pTemp, const TString& templateName)
{
    XMLElement *pChild = pFlagsElem->FirstChildElement("flag");

    while (pChild)
    {
        const char *pkMask = pChild->Attribute("mask");
        const char *pkName = pChild->Attribute("name");

        if (pkMask && pkName)
            pTemp->mBitFlags.push_back(CBitfieldTemplate::SBitFlag(pkName, TString(pkMask).ToInt32()));

        else
        {
            TString LogErrorBase = "Couldn't parse bit flag in " + templateName + "; ";

            if      (!pkMask && pkName) Log::Error(LogErrorBase + "no mask (" + pkName + ")");
            else if (pkMask && !pkName) Log::Error(LogErrorBase + "no name (mask " + pkMask + ")");
            else Log::Error(LogErrorBase + "no valid ID or name");
        }

        pChild = pChild->NextSiblingElement("flag");
    }
}

// ************ SCRIPT OBJECT ************
CScriptTemplate* CTemplateLoader::LoadScriptTemplate(XMLDocument *pDoc, const TString& rkTemplateName, u32 ObjectID)
{
    CScriptTemplate *pScript = new CScriptTemplate(mpMaster);
    pScript->mObjectID = ObjectID;
    pScript->mpBaseStruct = new CStructTemplate(-1, nullptr);

    XMLElement *pRoot = pDoc->FirstChildElement("ScriptTemplate");

    // Name
    XMLElement *pNameElem = pRoot->FirstChildElement("name");

    if (pNameElem)
    {
        pScript->mTemplateName = pNameElem->GetText();
        pScript->mpBaseStruct->SetName(pScript->mTemplateName);
    }

    // Properties
    XMLElement *pPropsElem = pRoot->FirstChildElement("properties");

    if (pPropsElem)
        LoadProperties(pPropsElem, pScript->mpBaseStruct, rkTemplateName);
    else
        Log::Error("Error reading script template " + rkTemplateName + "; there is no properties block");

    // Editor Parameters
    XMLElement *pEditor = pRoot->FirstChildElement("editor");

    if (pEditor)
    {
        // Editor Properties
        XMLElement *pEdProperties = pEditor->FirstChildElement("properties");
        XMLElement *pEdProp = pEdProperties->FirstChildElement("property");

        while (pEdProp)
        {
            TString Name = TString(pEdProp->Attribute("name")).ToLower();
            TString ID = TString(pEdProp->Attribute("ID")).ToLower();

            if (!Name.IsEmpty() && !ID.IsEmpty())
            {
                if (Name == "instancename")
                    pScript->mNameIDString = ID;
                else if (Name == "position")
                    pScript->mPositionIDString = ID;
                else if (Name == "rotation")
                    pScript->mRotationIDString = ID;
                else if (Name == "scale")
                    pScript->mScaleIDString = ID;
                else if (Name == "active")
                    pScript->mActiveIDString = ID;
                else if (Name == "lightparameters")
                    pScript->mLightParametersIDString = ID;
            }

            pEdProp = pEdProp->NextSiblingElement("property");
        }

        // Editor Assets
        XMLElement *pEdAssets = pEditor->FirstChildElement("assets");

        if (pEdAssets)
        {
            XMLElement *pAsset = pEdAssets->FirstChildElement();

            while (pAsset)
            {
                TString Type = TString(pAsset->Name()).ToLower();
                TString Source = TString(pAsset->Attribute("source")).ToLower();
                TString ID = pAsset->GetText();

                if (!Source.IsEmpty() && !ID.IsEmpty())
                {
                    CScriptTemplate::SEditorAsset Asset;

                    if (Type == "animparams")
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eAnimParams;
                    else if (Type == "model")
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eModel;
                    else if (Type == "billboard")
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eBillboard;
                    else if (Type == "collision")
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eCollision;
                    else
                    {
                        pAsset = pAsset->NextSiblingElement();
                        continue;
                    }

                    if (Source == "property")
                        Asset.AssetSource = CScriptTemplate::SEditorAsset::eProperty;
                    else if (Source == "file")
                        Asset.AssetSource = CScriptTemplate::SEditorAsset::eFile;
                    else
                    {
                        pAsset = pAsset->NextSiblingElement();
                        continue;
                    }

                    TString Force = pAsset->Attribute("force");

                    if (!Force.IsEmpty())
                        Asset.ForceNodeIndex = Force.ToInt32();
                    else
                        Asset.ForceNodeIndex = -1;

                    Asset.AssetLocation = ID;

                    // Validate property asset
                    if (Asset.AssetSource == CScriptTemplate::SEditorAsset::eProperty)
                    {
                        if (!pScript->mpBaseStruct->HasProperty(Asset.AssetLocation))
                        {
                            Log::Error("Error reading script template " + rkTemplateName + "; invalid property for " + Type + " asset: " + ID);
                            pAsset = pAsset->NextSiblingElement();
                            continue;
                        }
                    }

                    // Validate file asset
                    else
                    {
                        TString Path = "../resources/" + ID;
                        if (!boost::filesystem::exists(*Path))
                        {
                            Log::Error("Error reading script template " + rkTemplateName + "; invalid file for " + Type + " asset: " + ID);
                            pAsset = pAsset->NextSiblingElement();
                            continue;
                        }
                    }

                    pScript->mAssets.push_back(Asset);
                }

                pAsset = pAsset->NextSiblingElement();
            }
        }

        // Preview Scale
        XMLElement *pPreviewScale = pEditor->FirstChildElement("preview_scale");

        if (pPreviewScale)
            pScript->mPreviewScale = TString(pPreviewScale->GetText()).ToFloat();

        // Rotation
        XMLElement *pRotType = pEditor->FirstChildElement("rotation_type");

        if (pRotType)
        {
            TString RotType = TString(pRotType->GetText()).ToLower();

            if (!RotType.IsEmpty())
            {
                if (RotType == "disabled") pScript->mRotationType = CScriptTemplate::eRotationDisabled;
                else pScript->mRotationType = CScriptTemplate::eRotationEnabled;
            }
        }

        // Scale
        XMLElement *pScaleType = pEditor->FirstChildElement("scale_type");

        if (pScaleType)
        {
            TString ScaleType = TString(pScaleType->GetText()).ToLower();

            if (!ScaleType.IsEmpty())
            {
                if (ScaleType == "disabled") pScript->mScaleType = CScriptTemplate::eScaleDisabled;
                else if (ScaleType == "volume") pScript->mScaleType = CScriptTemplate::eScaleVolume;
                else pScript->mScaleType = CScriptTemplate::eScaleEnabled;
            }
        }

        // Preview Volume
        if (pScript->mScaleType == CScriptTemplate::eScaleVolume)
        {
            XMLElement *pVolume = pEditor->FirstChildElement("preview_volume");

            // Lambda to avoid duplicating volume shape code
            auto GetVolumeType = [](const TString& rkType) -> EVolumeShape {
                if (rkType == "none")           return eNoShape;
                if (rkType == "box")            return eBoxShape;
                if (rkType == "axisalignedbox") return eAxisAlignedBoxShape;
                if (rkType == "ellipsoid")      return eEllipsoidShape;
                if (rkType == "cylinder")       return eCylinderShape;
                if (rkType == "conditional")    return eConditionalShape;
                return eInvalidShape;
            };

            TString VolShape = TString(pVolume->Attribute("shape")).ToLower();

            if (!VolShape.IsEmpty())
                pScript->mVolumeShape = GetVolumeType(VolShape);

            TString VolScale = TString(pVolume->Attribute("scale")).ToLower();

            if (!VolScale.IsEmpty())
                pScript->mVolumeScale = VolScale.ToFloat();

            // Conditional
            if (pScript->mVolumeShape == eConditionalShape)
            {
                TString ConditionID = pVolume->Attribute("propertyID");

                if (!ConditionID.IsEmpty())
                {
                    pScript->mVolumeConditionIDString = ConditionID;
                    XMLElement *pCondition = pVolume->FirstChildElement("condition");

                    while (pCondition)
                    {
                        TString ConditionValue = TString(pCondition->Attribute("value")).ToLower();
                        TString ConditionShape = TString(pCondition->Attribute("shape")).ToLower();

                        if (!ConditionValue.IsEmpty() && !ConditionShape.IsEmpty())
                        {
                            CScriptTemplate::SVolumeCondition Condition;
                            Condition.Shape = GetVolumeType(ConditionShape);

                            if (ConditionValue == "true")
                                Condition.Value = 1;
                            else if (ConditionValue == "false")
                                Condition.Value = 0;
                            else
                                Condition.Value = ConditionValue.ToInt32(16);

                            TString ConditionScale = pCondition->Attribute("scale");
                            if (!ConditionScale.IsEmpty())
                                Condition.Scale = ConditionScale.ToFloat();
                            else
                                Condition.Scale = 1.f;

                            pScript->mVolumeConditions.push_back(Condition);
                        }

                        pCondition = pCondition->NextSiblingElement("condition");
                    }
                }
            }
        }
    }

    return pScript;
}

// ************ MASTER ************
void CTemplateLoader::LoadMasterTemplate(XMLDocument *pDoc, CMasterTemplate *pMaster)
{
    mpMaster = pMaster;
    mMasterDir = mskTemplatesDir + pMaster->mSourceFile.GetFileDirectory();

    XMLElement *pRoot = pDoc->FirstChildElement("MasterTemplate");
    mpMaster->mVersion = TString(pRoot->Attribute("version")).ToInt32();

    XMLElement *pElem = pRoot->FirstChildElement();

    while (pElem)
    {
        TString NodeName = pElem->Name();

        // Properties
        if (NodeName == "properties")
        {
            TString PropListPath = pElem->GetText();
            XMLDocument PropListXML;
            OpenXML(mMasterDir + PropListPath, PropListXML);

            if (!PropListXML.Error())
                LoadPropertyList(&PropListXML, PropListPath);
        }

        // Versions
        if (NodeName == "versions")
        {
            XMLElement *pVersion = pElem->FirstChildElement("version");

            while (pVersion)
            {
                mpMaster->mGameVersions.push_back(pVersion->GetText());
                pVersion = pVersion->NextSiblingElement("version");
            }
        }

        // Objects
        else if (NodeName == "objects")
        {
            XMLElement *pObj = pElem->FirstChildElement("object");

            while (pObj)
            {
                // ID can either be a hex number or an ASCII fourCC
                TString StrID = pObj->Attribute("ID");
                u32 ID;

                if (StrID.IsHexString(true))
                    ID = StrID.ToInt32();
                else
                    ID = CFourCC(StrID).ToLong();

                // Load up the object
                TString TemplateName = pObj->Attribute("template");

                XMLDocument ScriptXML;
                OpenXML(mMasterDir + TemplateName, ScriptXML);

                if (!ScriptXML.Error())
                {
                    CScriptTemplate *pTemp = LoadScriptTemplate(&ScriptXML, TemplateName, ID);

                    if (pTemp)
                    {
                        pTemp->mSourceFile = TemplateName;
                        mpMaster->mTemplates[ID] = pTemp;
                    }
                }

                pObj = pObj->NextSiblingElement("object");
            }
        }

        // States
        else if (NodeName == "states")
        {
            XMLElement *pState = pElem->FirstChildElement("state");

            while (pState)
            {
                TString StrID = pState->Attribute("ID");
                u32 StateID;

                if (StrID.IsHexString(true))
                    StateID = StrID.ToInt32();
                else
                    StateID = CFourCC(StrID).ToLong();

                TString StateName = pState->Attribute("name");
                mpMaster->mStates[StateID] = StateName;
                pState = pState->NextSiblingElement("state");
            }
        }

        // Messages
        else if (NodeName == "messages")
        {
            XMLElement *pMessage = pElem->FirstChildElement("message");

            while (pMessage)
            {
                TString StrID = pMessage->Attribute("ID");
                u32 MessageID;

                if (StrID.IsHexString(true))
                    MessageID = StrID.ToInt32();
                else
                    MessageID = CFourCC(StrID).ToLong();

                TString MessageName = pMessage->Attribute("name");
                mpMaster->mMessages[MessageID] = MessageName;
                pMessage = pMessage->NextSiblingElement("message");
            }
        }

        pElem = pElem->NextSiblingElement();
    }

    pMaster->mFullyLoaded = true;
}

void CTemplateLoader::LoadPropertyList(XMLDocument *pDoc, const TString& ListName)
{
    XMLElement *pRootElem = pDoc->FirstChildElement("Properties");

    if (!pRootElem)
        Log::Error("Error reading property list at " + ListName + "; there is no root \"Properties\" block element");

    else
    {
        XMLElement *pElem = pRootElem->FirstChildElement("property");

        while (pElem)
        {
            TString ID = pElem->Attribute("ID");
            TString Name = pElem->Attribute("name");

            if (!ID.IsEmpty() && !Name.IsEmpty())
                mpMaster->mPropertyNames[ID.ToInt32()] = Name;

            pElem = pElem->NextSiblingElement();
        }

        mpMaster->mHasPropList = true;
    }
}

CMasterTemplate* CTemplateLoader::LoadGameInfo(XMLNode *pNode)
{
    CMasterTemplate *pMaster = new CMasterTemplate();
    XMLElement *pGameElem = pNode->FirstChildElement();

    // Parse game parameters
    while (pGameElem)
    {
        TString NodeName = TString(pGameElem->Name()).ToLower();

        if (NodeName == "name")
            pMaster->mGameName = pGameElem->GetText();

        else if (NodeName == "mrea")
        {
            u32 VersionNum = std::stoul(pGameElem->GetText(), 0, 16);
            pMaster->mGame = CAreaLoader::GetFormatVersion(VersionNum);
        }

        else if (NodeName == "master")
        {
            TString MasterPath = pGameElem->GetText();
            pMaster->mSourceFile = MasterPath;
        }

        pGameElem = pGameElem->NextSiblingElement();
    }

    return pMaster;
}

// ************ UTILITY ************
void CTemplateLoader::OpenXML(const TString& rkPath, XMLDocument& rDoc)
{
    TString AbsPath = rkPath;
    rDoc.LoadFile(*AbsPath);

    if (rDoc.Error())
    {
        TString Name = AbsPath.GetFileName();
        Log::Error("Error when opening template XML " + Name + ": " + ErrorName(rDoc.ErrorID()));
    }
}

TString CTemplateLoader::ErrorName(XMLError Error)
{
    switch (Error)
    {
    case XML_SUCCESS: return "Success";
    case XML_NO_ATTRIBUTE: return "No attribute";
    case XML_WRONG_ATTRIBUTE_TYPE: return "Wrong attribute type";
    case XML_ERROR_FILE_NOT_FOUND: return "File not found";
    case XML_ERROR_FILE_COULD_NOT_BE_OPENED: return "File could not be opened";
    case XML_ERROR_FILE_READ_ERROR: return "File read error";
    case XML_ERROR_ELEMENT_MISMATCH: return "Element mismatch";
    case XML_ERROR_PARSING_ELEMENT: return "Parsing element";
    case XML_ERROR_PARSING_ATTRIBUTE: return "Parsing attribute";
    case XML_ERROR_IDENTIFYING_TAG: return "Identifying tag";
    case XML_ERROR_PARSING_TEXT: return "Parsing text";
    case XML_ERROR_PARSING_CDATA: return "Parsing CData";
    case XML_ERROR_PARSING_COMMENT: return "Parsing comment";
    case XML_ERROR_PARSING_DECLARATION: return "Parsing declaration";
    case XML_ERROR_PARSING_UNKNOWN: return "Parsing unknown";
    case XML_ERROR_EMPTY_DOCUMENT: return "Empty document";
    case XML_ERROR_MISMATCHED_ELEMENT: return "Mismatched element";
    case XML_ERROR_PARSING: return "Parsing";
    case XML_CAN_NOT_CONVERT_TEXT: return "Cannot convert text";
    case XML_NO_TEXT_NODE: return "No text node";
    default: return "Unknown error";
    }
}

// ************ PUBLIC ************
void CTemplateLoader::LoadGameList()
{
    Log::Write("Loading game list");

    // Load Game List XML
    XMLDocument GameListXML;
    OpenXML(mskGameListPath, GameListXML);

    if (GameListXML.Error())
        return;

    // Parse
    XMLNode *pNode = GameListXML.FirstChild()->NextSibling()->FirstChild();

    while (pNode)
    {
        XMLElement *pElement = pNode->ToElement();
        TString NodeName = TString(pElement->Name()).ToLower();

        // Game List version number
        if (NodeName == "version")
        {
            u32 VersionNum = std::stoul(pElement->GetText());
            CMasterTemplate::smGameListVersion = VersionNum;
        }

        // Games
        else if (NodeName == "game")
        {
            CTemplateLoader Loader(mskTemplatesDir);
            CMasterTemplate *pMaster = Loader.LoadGameInfo(pNode);
            CMasterTemplate::smMasterMap[pMaster->mGame] = pMaster;
        }

        pNode = pNode->NextSibling();
    }
}

void CTemplateLoader::LoadGameTemplates(EGame Game)
{
    std::list<CMasterTemplate*> MasterList = CMasterTemplate::GetMasterList();

    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        if (pMaster->GetGame() == Game)
        {
            XMLDocument MasterXML;
            OpenXML(mskTemplatesDir + pMaster->mSourceFile, MasterXML);

            if (!MasterXML.Error())
            {
                CTemplateLoader Loader(mskTemplatesDir);
                Loader.LoadMasterTemplate(&MasterXML, pMaster);
            }

            break;
        }
    }
}
