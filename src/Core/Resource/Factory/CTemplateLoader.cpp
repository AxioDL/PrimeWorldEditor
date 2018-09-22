#include "CTemplateLoader.h"
#include "CAreaLoader.h"
#include "Core/Resource/Script/Property/Properties.h"
#include <Common/FileUtil.h>
#include <Common/Log.h>

const TString CTemplateLoader::mskTemplatesDir = "../templates/";
const TString CTemplateLoader::mskGameListPath = CTemplateLoader::mskTemplatesDir + "GameList.xml";

using namespace tinyxml2;

// ugly macro because this is all temp code anyway so whatever
#define SET_MEMBER_CASES_NUMERICAL(MemberName, Param, LParam) \
    case EPropertyTypeNew::Byte:\
        TPropCast<CByteProperty>(pProp)->MemberName = (s8) LParam.ToInt32(10);\
        break;\
        \
    case EPropertyTypeNew::Short:\
        TPropCast<CShortProperty>(pProp)->MemberName = (s16) LParam.ToInt32(10);\
        break;\
        \
    case EPropertyTypeNew::Int:\
        TPropCast<CIntProperty>(pProp)->MemberName = (s32) LParam.ToInt32(10);\
        break;\
        \
    case EPropertyTypeNew::Float:\
        TPropCast<CFloatProperty>(pProp)->MemberName = LParam.ToFloat();\
        break;\
        \

#define SET_MEMBER_CASES_NON_NUMERICAL(MemberName, Param, LParam) \
    case EPropertyTypeNew::Bool:\
        TPropCast<CBoolProperty>(pProp)->MemberName = (LParam == "true");\
        break;\
        \
    case EPropertyTypeNew::Choice:\
        TPropCast<CChoiceProperty>(pProp)->MemberName = LParam.ToInt32( LParam.StartsWith("0x") ? 16 : 10 );\
        break;\
        \
    case EPropertyTypeNew::Enum:\
        TPropCast<CEnumProperty>(pProp)->MemberName = LParam.ToInt32( LParam.StartsWith("0x") ? 16 : 10 );\
        break;\
        \
    case EPropertyTypeNew::Flags:\
        TPropCast<CFlagsProperty>(pProp)->MemberName = LParam.ToInt32( LParam.StartsWith("0x") ? 16 : 10 );\
        break;\
        \
    case EPropertyTypeNew::String:\
        TPropCast<CStringProperty>(pProp)->MemberName = Param;\
        break;\
        \
    case EPropertyTypeNew::Vector:\
    {\
        TStringList Components = Param.Split(", ");\
        if (Components.size() != 3) {\
            TPropCast<CVectorProperty>(pProp)->MemberName = CVector3f::skInfinite;\
            break;\
        }\
        float* pPtr = &TPropCast<CVectorProperty>(pProp)->MemberName.X;\
        for (auto it = Components.begin(); it != Components.end(); it++)\
        {\
            *pPtr = it->ToFloat();\
            pPtr++;\
        }\
        break;\
    }\
    case EPropertyTypeNew::Color:\
    {\
        TStringList Components = Param.Split(", ");\
        if (Components.size() < 3 || Components.size() > 4) {\
            TPropCast<CColorProperty>(pProp)->MemberName = CColor::skTransparentBlack;\
            break;\
        }\
        float* pPtr = &TPropCast<CColorProperty>(pProp)->MemberName.R;\
        TPropCast<CColorProperty>(pProp)->MemberName.A = 1.0f;\
        for (auto it = Components.begin(); it != Components.end(); it++) {\
            *pPtr = it->ToFloat();\
            pPtr++;\
        }\
        break;\
    }\
    case EPropertyTypeNew::Asset:\
        TPropCast<CAssetProperty>(pProp)->MemberName = CAssetID::FromString(Param);\
        break;\
        \
    case EPropertyTypeNew::Sound:\
        TPropCast<CSoundProperty>(pProp)->MemberName = LParam.ToInt32(10);\
        break;\
        \

#define SET_MEMBER_FROM_STRING_TYPED(MemberName, Param, LParam)\
    switch (Type)\
    {\
    SET_MEMBER_CASES_NON_NUMERICAL(MemberName, Param, LParam)\
    SET_MEMBER_CASES_NUMERICAL(MemberName, Param, LParam)\
    default:\
        ASSERT(false);\
        break;\
    }\

#define SET_MEMBER_FROM_STRING_NUMERICAL(MemberName, Param, LParam)\
    switch(Type)\
    {\
    SET_MEMBER_CASES_NUMERICAL(MemberName, Param, LParam)\
    default:\
        ASSERT(false);\
        break;\
    }\

EPropertyTypeNew PropStringToPropEnum(TString Prop)
{
    Prop = Prop.ToLower();
    if (Prop == "bool")       return EPropertyTypeNew::Bool;
    if (Prop == "byte")       return EPropertyTypeNew::Byte;
    if (Prop == "short")      return EPropertyTypeNew::Short;
    if (Prop == "long")       return EPropertyTypeNew::Int;
    if (Prop == "enum")       return EPropertyTypeNew::Enum;
    if (Prop == "bitfield")   return EPropertyTypeNew::Flags;
    if (Prop == "float")      return EPropertyTypeNew::Float;
    if (Prop == "string")     return EPropertyTypeNew::String;
    if (Prop == "color")      return EPropertyTypeNew::Color;
    if (Prop == "vector3f")   return EPropertyTypeNew::Vector;
    if (Prop == "sound")      return EPropertyTypeNew::Sound;
    if (Prop == "asset")      return EPropertyTypeNew::Asset;
    if (Prop == "struct")     return EPropertyTypeNew::Struct;
    if (Prop == "array")      return EPropertyTypeNew::Array;
    if (Prop == "character")  return EPropertyTypeNew::AnimationSet;
    if (Prop == "mayaspline") return EPropertyTypeNew::Spline;
                              return EPropertyTypeNew::Invalid;
}

IPropertyNew* CTemplateLoader::LoadProperty(XMLElement* pElem, CScriptTemplate* pScript, CStructPropertyNew* pParent, const TString& rkTemplateName)
{
    TString NodeType = TString(pElem->Name()).ToLower();
    TString IDAttr = TString(pElem->Attribute("ID")).ToLower();
    TString TypeAttr = TString(pElem->Attribute("type")).ToLower();
    TString NameAttr = pElem->Attribute("name");
    TString TemplateAttr = pElem->Attribute("template");

    // Get ID + name
    if (IDAttr.IsEmpty())
    {
        Log::Error(rkTemplateName + ": ran into a property with no ID");
        return nullptr;
    }

    u32 ID = IDAttr.ToInt32();
    TString Name;

    if (!NameAttr.IsEmpty())
        Name = NameAttr;
    else if (mGame >= eEchoesDemo)
        Name = CMasterTemplate::PropertyName(ID);
    else
    {
        Log::Error(rkTemplateName + ": Property " + TString::HexString(ID) + " doesn't have a name either in the template itself nor in the master list");
        return nullptr;
    }

    // Does the property already exist (eg is this an override)?
    IPropertyNew* pProp = pParent->ChildByID(ID);
    EPropertyTypeNew Type;
    bool IsNewProperty = false;

    // If it doesn't, then we'll need to create it.
    if (!pProp)
    {
        // Determine type
        TString TypeStr = (NodeType == "property" ? TypeAttr : NodeType);
        Type = PropStringToPropEnum(TypeStr);
        IsNewProperty = true;

        if (Type == EPropertyTypeNew::Invalid)
        {
            if (TypeStr.IsEmpty())
                Log::Error(rkTemplateName + ": Property " + TString::HexString(ID) + " doesn't have a type set");
            else
                Log::Error(rkTemplateName + ": Property " + TString::HexString(ID) + " has an invalid type set: " + TypeStr);

            return nullptr;
        }

        // Load archetype if required
        bool bNeedsArchetype = ( Type == EPropertyTypeNew::Struct ||
                                 Type == EPropertyTypeNew::Enum ||
                                 Type == EPropertyTypeNew::Choice ||
                                 Type == EPropertyTypeNew::Flags );

        if (bNeedsArchetype)
        {
            IPropertyNew* pArchetype = nullptr;

            //todo: struct archetypes are not supposed to be optional but apparently some still don't have them
            if (!TemplateAttr.IsEmpty())
            {
                if (Type == EPropertyTypeNew::Struct)
                {
                    pArchetype = LoadStructArchetype(TemplateAttr);
                }
                else if (Type == EPropertyTypeNew::Enum || Type == EPropertyTypeNew::Choice)
                {
                    pArchetype = LoadEnumArchetype(TemplateAttr, Type == EPropertyTypeNew::Choice);
                }
                else if (Type == EPropertyTypeNew::Flags)
                {
                    pArchetype = LoadFlagsArchetype(TemplateAttr);
                }
            }

            // create property as a copy of the archetype
            if (pArchetype != nullptr)
            {
                pProp = IPropertyNew::CreateCopy(pArchetype);
            }
        }

        // no archetype, so do normal create
        if (!pProp)
        {
            pProp = IPropertyNew::Create(Type, mGame);
        }

        // we need to have a valid property by this point
        if (!pProp)
        {
            Log::Error(rkTemplateName + ": Property " + TString::HexString(ID) + " seems to be using a valid but unsupported property type? (" + TypeStr + ")");
            return nullptr;
        }

        // Initialize parameters on the new property
        pProp->mID = ID;
        pProp->mName = Name;
    }
    else
        Type = pProp->Type();

    // Common parameters
    XMLElement *pParams = pElem->FirstChildElement();

    while (pParams)
    {
        TString ParamName = TString(pParams->Name()).ToLower();
        TString ParamVal = TString(pParams->GetText());

#if 0
        // Load versions
        if (ParamName == "versions")
        {
            XMLElement *pVersion = pParams->FirstChildElement("version");

            while (pVersion)
            {
                TString VerName = pVersion->GetText();
                u32 VerIdx = mpMaster->GameVersion(VerName);

                if (VerIdx == -1)
                    Log::Error(rkTemplateName + ": Property " + TString::HexString(ID) + " has invalid version \"" + VerName + "\"");
                else
                    pProp->mAllowedVersions.push_back(VerIdx);

                pVersion = pVersion->NextSiblingElement("version");
            }
        }

        // Otherwise, delegate it to the template to parse the parameter.
        // (This is done because there's no common base class for typed properties, so it's tough to handle this in the template loader.)
        else
            Prop->SetParam(ParamName, ParamVal);
#endif
        if (ParamName == "cook_pref")
        {
            TString lValue = ParamVal.ToLower();

            if (lValue == "always")
                pProp->mCookPreference = ECookPreferenceNew::Always;
            else if (lValue == "never")
                pProp->mCookPreference = ECookPreferenceNew::Never;
            else
                pProp->mCookPreference = ECookPreferenceNew::Default;
        }
        else if (ParamName == "description")
        {
            pProp->mDescription = ParamVal;
        }
        else if (ParamName == "default")
        {
            TString lValue = ParamVal.ToLower();
            SET_MEMBER_FROM_STRING_TYPED(mDefaultValue, ParamVal, lValue);
        }
        else if (ParamName == "range")
        {
            TStringList Components = ParamVal.ToLower().Split(", ");
            TString Min = Components.front();
            TString Max = Components.back();
            SET_MEMBER_FROM_STRING_NUMERICAL(mMinValue, Min, Min);
            SET_MEMBER_FROM_STRING_NUMERICAL(mMaxValue, Max, Max);
        }
        else if (ParamName == "suffix")
        {
            pProp->SetSuffix(ParamVal);
        }

        pParams = pParams->NextSiblingElement();
    }

    // Asset-specific parameters
    if (Type == EPropertyTypeNew::Asset)
    {
        TString ExtensionsAttr = pElem->Attribute("extensions");

        if (!ExtensionsAttr.IsEmpty())
        {
            TStringList ExtensionsList = ExtensionsAttr.Split(", ");
            CAssetProperty* pAsset = TPropCast<CAssetProperty>(pProp);
            pAsset->SetTypeFilter(ExtensionsList);
        }
    }

    // Enum-specific parameters
    else if (Type == EPropertyTypeNew::Enum || Type == EPropertyTypeNew::Choice)
    {
        // use static_cast so we can do both enum and choice with this code
        CEnumProperty* pEnum = static_cast<CEnumProperty*>(pProp);

        // Load embedded enumerators
        XMLElement* pEnumerators = pElem->FirstChildElement("enumerators");

        if (pEnumerators)
            LoadEnumerators(pEnumerators, pEnum, rkTemplateName);
    }

    // Bitfield-specific parameters
    else if (Type == EPropertyTypeNew::Flags)
    {
        CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);

        // Load embedded flags
        XMLElement* pFlagsElem = pElem->FirstChildElement("flags");

        if (pFlagsElem)
            LoadBitFlags(pFlagsElem, pFlags, rkTemplateName);
    }

    // Struct-specific parameters
    else if ( (Type == EPropertyTypeNew::Struct) || (Type == EPropertyTypeNew::Array) )
    {
        CStructPropertyNew* pStruct = nullptr;

        if (Type == EPropertyTypeNew::Struct)
        {
            pStruct = TPropCast<CStructPropertyNew>(pProp);
        }
        else
        {
            CArrayProperty* pArray = TPropCast<CArrayProperty>(pProp);

            if (pArray->mpItemArchetype != nullptr)
            {
                ASSERT(pArray->mpItemArchetype->Type() == EPropertyTypeNew::Struct);
                pStruct = TPropCast<CStructPropertyNew>(pArray->mpItemArchetype);
            }
            else
            {
                if (Name == "Activation Times")
                {
                    CFloatProperty* pFloatItem = (CFloatProperty*) IPropertyNew::Create(EPropertyTypeNew::Float, mGame);
                    pFloatItem->mName = "Time";
                    pFloatItem->mID = 0;
                    pFloatItem->mDefaultValue = 0.0f;
                    pFloatItem->mFlags = EPropertyFlag::IsArrayArchetype;
                    pArray->mpItemArchetype = pFloatItem;
                }
                else
                {
                    pArray->mpItemArchetype = IPropertyNew::Create(EPropertyTypeNew::Struct, mGame);
                    pStruct = TPropCast<CStructPropertyNew>(pArray->mpItemArchetype);
                    pStruct->mFlags = EPropertyFlag::IsAtomic | EPropertyFlag::IsArrayArchetype;
                }
            }

            XMLElement* pItemNameElem = pElem->FirstChildElement("element_name");

            if (pStruct && pItemNameElem)
                pStruct->mName = pItemNameElem->GetText();
        }

        // Load parameter overrides
        XMLElement *pProperties = pElem->FirstChildElement("properties");

        if (pStruct && pProperties)
        {
            LoadProperties(pProperties, pScript, pStruct, rkTemplateName);
        }
    }

    if (IsNewProperty)
    {
        CMasterTemplate::AddProperty(pProp, mMasterDir + rkTemplateName);

        if (pParent)
            pParent->mChildren.push_back(pProp);
    }

    return pProp;
}

CStructPropertyNew* CTemplateLoader::LoadStructArchetype(const TString& rkTemplateFileName)
{
    // Check whether this struct has already been read
    TString StructName = rkTemplateFileName.GetFileName(false);
    CStructPropertyNew* pArchetype = static_cast<CStructPropertyNew*>( mpMaster->FindPropertyArchetype(StructName) );

    // Names cannot be shared between multiple property archetypes
    if (pArchetype != nullptr)
    {
        ASSERT(pArchetype->Type() == EPropertyTypeNew::Struct);
    }

    // If the struct template hasn't been read yet, then we read it and add it to master's list
    if (!pArchetype)
    {
        XMLDocument Doc;
        OpenXML(mskTemplatesDir + mMasterDir + rkTemplateFileName, Doc);

        if (!Doc.Error())
        {
            pArchetype = TPropCast<CStructPropertyNew>(
                        IPropertyNew::Create(EPropertyTypeNew::Struct, mGame)
                        );
            ASSERT(pArchetype != nullptr);

            XMLElement* pRootElem = Doc.FirstChildElement("struct");
            ASSERT(pRootElem);

            TString TypeAttr = TString(pRootElem->Attribute("type")).ToLower();
            ASSERT(!TypeAttr.IsEmpty())

            if (TypeAttr == "single")
                pArchetype->mFlags |= EPropertyFlag::IsAtomic;

            pArchetype->mFlags |= EPropertyFlag::IsArchetype;
            pArchetype->mTemplateFileName = rkTemplateFileName;
            pArchetype->mName = rkTemplateFileName.GetFileName(false);

#if 0
            if (pArchetype->mTypeName.Contains("Struct"))
            {
                pArchetype->mSourceFile = "Structs/" +
                        GetGameShortName(mGame) +
                        "-" +
                        pArchetype->mTypeName +
                        ".xml";
                pArchetype->mTypeName = pArchetype->mSourceFile.GetFileName(false);
            }
#endif

            // ignore struct name attribute - archetypes should always have the type name
#if 0
            TString NameAttr = TString(pRootElem->Attribute("name"));
            if (!NameAttr.IsEmpty())
                pArchetype->mName = NameAttr;
#endif

            // Read sub-properties
            XMLElement* pSubPropsElem = pRootElem->FirstChildElement("properties");
            ASSERT(pSubPropsElem);

            LoadProperties(pSubPropsElem, nullptr, pArchetype, rkTemplateFileName);
            pArchetype->Initialize(nullptr, nullptr, 0);

            mpMaster->mPropertyTemplates.emplace(
                        std::make_pair(
                                StructName,
                                CMasterTemplate::SPropertyTemplatePath(rkTemplateFileName, pArchetype)
                            ));
        }
    }

    ASSERT(pArchetype != nullptr);
    return pArchetype;
}

CEnumProperty* CTemplateLoader::LoadEnumArchetype(const TString& rkTemplateFileName, bool bIsChoice)
{
    // Check whether this struct has already been read
    TString EnumName = rkTemplateFileName.GetFileName(false);
    CEnumProperty* pArchetype = static_cast<CEnumProperty*>( mpMaster->FindPropertyArchetype(EnumName) );

    // Names cannot be shared between multiple property archetypes
    if (pArchetype != nullptr)
    {
        ASSERT(pArchetype->Type() == EPropertyTypeNew::Enum || pArchetype->Type() == EPropertyTypeNew::Choice);
    }

    // If the enum template hasn't been read yet, then we read it and add it to master's list
    if (!pArchetype)
    {
        XMLDocument Doc;
        OpenXML(mskTemplatesDir + mMasterDir + rkTemplateFileName, Doc);

        if (!Doc.Error())
        {
            // use static_cast so this code works for both enum and choice
            pArchetype = static_cast<CEnumProperty*>(
                        IPropertyNew::Create(bIsChoice ? EPropertyTypeNew::Choice : EPropertyTypeNew::Enum, mGame)
                        );
            ASSERT(pArchetype != nullptr);

            pArchetype->mName = rkTemplateFileName.GetFileName(false);
            pArchetype->mFlags |= EPropertyFlag::IsArchetype;
            pArchetype->mSourceFile = rkTemplateFileName;

            XMLElement* pRootElem = Doc.FirstChildElement("enum");
            ASSERT(pRootElem);

            XMLElement *pEnumers = pRootElem->FirstChildElement("enumerators");
            ASSERT(pEnumers);

            LoadEnumerators(pEnumers, pArchetype, rkTemplateFileName);
            pArchetype->Initialize(nullptr, nullptr, 0);

            mpMaster->mPropertyTemplates.emplace(
                        std::make_pair(
                            EnumName,
                            CMasterTemplate::SPropertyTemplatePath(rkTemplateFileName, pArchetype)
                        ));
        }
    }

    ASSERT(pArchetype != nullptr);
    return pArchetype;
}

CFlagsProperty* CTemplateLoader::LoadFlagsArchetype(const TString& rkTemplateFileName)
{
    // Check whether this struct has already been read
    TString FlagsName = rkTemplateFileName.GetFileName(false);
    CFlagsProperty* pArchetype = static_cast<CFlagsProperty*>( mpMaster->FindPropertyArchetype(FlagsName) );

    // Names cannot be shared between multiple property archetypes
    if (pArchetype != nullptr)
    {
        ASSERT(pArchetype->Type() == EPropertyTypeNew::Flags);
    }

    // If the enum template hasn't been read yet, then we read it and add it to master's list
    if (!pArchetype)
    {
        XMLDocument Doc;
        OpenXML(mskTemplatesDir + mMasterDir + rkTemplateFileName, Doc);

        if (!Doc.Error())
        {
            pArchetype = TPropCast<CFlagsProperty>(
                        IPropertyNew::Create(EPropertyTypeNew::Flags, mGame)
                        );
            ASSERT(pArchetype != nullptr);

            pArchetype->mName = rkTemplateFileName.GetFileName(false);
            pArchetype->mFlags |= EPropertyFlag::IsArchetype;
            pArchetype->mSourceFile = rkTemplateFileName;

            XMLElement *pRootElem = Doc.FirstChildElement("bitfield");
            ASSERT(pRootElem);

            XMLElement *pFlags = pRootElem->FirstChildElement("flags");
            ASSERT(pFlags);

            LoadBitFlags(pFlags, pArchetype, rkTemplateFileName);
            pArchetype->Initialize(nullptr, nullptr, 0);

            mpMaster->mPropertyTemplates.emplace(
                        std::make_pair(
                            FlagsName,
                            CMasterTemplate::SPropertyTemplatePath(rkTemplateFileName, pArchetype)
                        ));

        }
    }

    ASSERT(pArchetype != nullptr);
    return pArchetype;
}

void CTemplateLoader::LoadProperties(XMLElement *pPropertiesElem, CScriptTemplate *pScript, CStructPropertyNew* pStruct, const TString& rkTemplateName)
{
    XMLElement *pChild = pPropertiesElem->FirstChildElement();

    while (pChild)
    {
        TString NodeType = TString(pChild->Name()).ToLower();

        if ( (NodeType != "property") && (NodeType != "struct") && (NodeType != "enum") && (NodeType != "bitfield") && (NodeType != "array") )
        {
            Log::Error(rkTemplateName + ": A node in a properties block has an invalid name: " + NodeType);
        }

        // LoadProperty adds newly created properties to the struct, so we don't need to do anything other than call it for each sub-element.
        else
        {
            LoadProperty(pChild, pScript, pStruct, rkTemplateName);
        }

        pChild = pChild->NextSiblingElement();
    }
}

void CTemplateLoader::LoadEnumerators(XMLElement *pEnumeratorsElem, CEnumProperty* pEnum, const TString& rkTemplateName)
{
    XMLElement *pChild = pEnumeratorsElem->FirstChildElement("enumerator");

    while (pChild)
    {
        const char *pkID = pChild->Attribute("ID");
        const char *pkName = pChild->Attribute("name");

        if (pkID && pkName)
        {
            u32 EnumeratorID = TString(pkID).ToInt32();
            pEnum->mValues.push_back(CEnumProperty::SEnumValue(pkName, EnumeratorID));
        }

        else
        {
            TString LogErrorBase = rkTemplateName + ": Couldn't parse enumerator; ";

            if      (!pkID && pkName) Log::Error(LogErrorBase + "no valid ID (" + pkName + ")");
            else if (pkID && !pkName) Log::Error(LogErrorBase + "no valid name (ID " + pkID + ")");
            else Log::Error(LogErrorBase + "no valid ID or name");
        }

        pChild = pChild->NextSiblingElement("enumerator");
    }
}

void CTemplateLoader::LoadBitFlags(XMLElement *pFlagsElem, CFlagsProperty* pFlags, const TString& kTemplateName)
{
    XMLElement *pChild = pFlagsElem->FirstChildElement("flag");

    while (pChild)
    {
        const char *pkMask = pChild->Attribute("mask");
        const char *pkName = pChild->Attribute("name");

        if (pkMask && pkName)
            pFlags->mBitFlags.push_back(CFlagsProperty::SBitFlag(pkName, TString(pkMask).ToInt32()));

        else
        {
            TString LogErrorBase = kTemplateName + ": Couldn't parse bit flag; ";

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
    pScript->mSourceFile = rkTemplateName;

    IPropertyNew* pBaseStruct = IPropertyNew::Create(EPropertyTypeNew::Struct, mGame);
    pScript->mpProperties = std::make_unique<CStructPropertyNew>( *TPropCast<CStructPropertyNew>(pBaseStruct) );

    XMLElement *pRoot = pDoc->FirstChildElement("ScriptTemplate");

    // Name
    XMLElement *pNameElem = pRoot->FirstChildElement("name");
    ASSERT(pNameElem);
    pScript->mpProperties->mName = pNameElem->GetText();

    // Modules
    XMLElement *pModulesElem = pRoot->FirstChildElement("modules");

    if (pModulesElem)
    {
        XMLElement *pModuleElem = pModulesElem->FirstChildElement("module");

        while (pModuleElem)
        {
            pScript->mModules.push_back(pModuleElem->GetText());
            pModuleElem = pModuleElem->NextSiblingElement("module");
        }
    }

    // Properties
    XMLElement *pPropsElem = pRoot->FirstChildElement("properties");

    if (pPropsElem)
        LoadProperties(pPropsElem, pScript, pScript->Properties(), rkTemplateName);
    else
        Log::Error(rkTemplateName + ": There is no \"properties\" block element");

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
            TString ID = TString(pEdProp->Attribute("ID"));

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
                    TStringList AcceptedExtensions;

                    if (Type == "animparams")
                    {
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eAnimParams;
                        AcceptedExtensions.push_back("ANCS");
                        AcceptedExtensions.push_back("CHAR");
                    }

                    else if (Type == "model")
                    {
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eModel;
                        AcceptedExtensions.push_back("CMDL");
                    }

                    else if (Type == "billboard")
                    {
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eBillboard;
                        AcceptedExtensions.push_back("TXTR");
                    }

                    else if (Type == "collision")
                    {
                        Asset.AssetType = CScriptTemplate::SEditorAsset::eCollision;
                        AcceptedExtensions.push_back("DCLN");
                    }

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
                        if (!pScript->mpProperties->ChildByIDString(Asset.AssetLocation))
                        {
                            Log::Error(rkTemplateName + ": Invalid property for " + Type + " asset: " + ID);
                            pAsset = pAsset->NextSiblingElement();
                            continue;
                        }
                    }

                    // Validate file asset
                    else
                    {
                        CResourceEntry *pEntry = gpEditorStore->FindEntry(ID);

                        if (!pEntry)
                        {
                            Log::Error(rkTemplateName + ": Invalid file for " + Type + " asset: " + ID);
                            pAsset = pAsset->NextSiblingElement();
                            continue;
                        }
                    }

                    pScript->mAssets.push_back(Asset);
                }

                pAsset = pAsset->NextSiblingElement();
            }
        }

        // Attachments
        XMLElement *pAttachments = pEditor->FirstChildElement("attachments");

        if (pAttachments)
        {
            XMLElement *pAttachment = pAttachments->FirstChildElement("attachment");
            u32 AttachIdx = 0;

            while (pAttachment)
            {
                SAttachment Attachment;
                Attachment.AttachProperty = pAttachment->Attribute("propertyID");
                Attachment.LocatorName = pAttachment->Attribute("locator");
                Attachment.AttachType = eAttach;

                // Validate property
                IPropertyNew* pProp = pScript->mpProperties->ChildByIDString(Attachment.AttachProperty);

                if (!pProp)
                    Log::Error(rkTemplateName + ": Invalid property for attachment " + TString::FromInt32(AttachIdx) + ": " + Attachment.AttachProperty);
                else if (pProp->Type() != EPropertyTypeNew::AnimationSet && (pProp->Type() != EPropertyTypeNew::Asset || !TPropCast<CAssetProperty>(pProp)->GetTypeFilter().Accepts(eModel)))
                    Log::Error(rkTemplateName + ": Property referred to by attachment " + TString::FromInt32(AttachIdx) + " is not an attachable asset! Must be a file property that accepts CMDLs, or an animation set property.");

                else
                {
                    // Check sub-elements
                    XMLElement *pParams = pAttachment->FirstChildElement();

                    while (pParams)
                    {
                        TString ParamName = pParams->Name();

                        if (ParamName == "attach_type")
                        {
                            TString Type = TString(pParams->GetText()).ToLower();
                            if (Type == "follow") Attachment.AttachType = eFollow;
                            else if (Type != "attach") Log::Error(rkTemplateName + ": Attachment " + TString::FromInt32(AttachIdx) + " has invalid attach type specified: " + Type);
                        }

                        pParams = pParams->NextSiblingElement();
                    }

                    pScript->mAttachments.push_back(Attachment);
                }

                pAttachment = pAttachment->NextSiblingElement("attachment");
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

    pScript->PostLoad();
    return pScript;
}

// ************ MASTER ************
void CTemplateLoader::LoadMasterTemplate(XMLDocument *pDoc, CMasterTemplate *pMaster)
{
    mpMaster = pMaster;
    mMasterDir = pMaster->mSourceFile.GetFileDirectory();

    XMLElement *pRoot = pDoc->FirstChildElement("MasterTemplate");
    XMLElement *pElem = pRoot->FirstChildElement();

    while (pElem)
    {
        TString NodeName = pElem->Name();

        // Objects
        if (NodeName == "objects")
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
                OpenXML(mskTemplatesDir + mMasterDir + TemplateName, ScriptXML);

                if (!ScriptXML.Error())
                {
                    CScriptTemplate *pTemp = LoadScriptTemplate(&ScriptXML, TemplateName, ID);

                    if (pTemp)
                    {
                        mpMaster->mScriptTemplates.emplace(
                                    std::make_pair(
                                        ID,
                                        CMasterTemplate::SScriptTemplatePath(ID, TemplateName, pTemp)
                                    ));
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
        Log::Error("Error opening " + Name + ": " + ErrorName(rDoc.ErrorID()));
    }
}

TString CTemplateLoader::ErrorName(XMLError Error)
{
    switch (Error)
    {
    case XML_SUCCESS:                           return "Success";
    case XML_NO_ATTRIBUTE:                      return "No attribute";
    case XML_WRONG_ATTRIBUTE_TYPE:              return "Wrong attribute type";
    case XML_ERROR_FILE_NOT_FOUND:              return "File not found";
    case XML_ERROR_FILE_COULD_NOT_BE_OPENED:    return "File could not be opened";
    case XML_ERROR_FILE_READ_ERROR:             return "File read error";
    case XML_ERROR_PARSING_ELEMENT:             return "Parsing element";
    case XML_ERROR_PARSING_ATTRIBUTE:           return "Parsing attribute";
    case XML_ERROR_PARSING_TEXT:                return "Parsing text";
    case XML_ERROR_PARSING_CDATA:               return "Parsing CData";
    case XML_ERROR_PARSING_COMMENT:             return "Parsing comment";
    case XML_ERROR_PARSING_DECLARATION:         return "Parsing declaration";
    case XML_ERROR_PARSING_UNKNOWN:             return "Parsing unknown";
    case XML_ERROR_EMPTY_DOCUMENT:              return "Empty document";
    case XML_ERROR_MISMATCHED_ELEMENT:          return "Mismatched element";
    case XML_ERROR_PARSING:                     return "Parsing";
    case XML_CAN_NOT_CONVERT_TEXT:              return "Cannot convert text";
    case XML_NO_TEXT_NODE:                      return "No text node";
    case XML_ELEMENT_DEPTH_EXCEEDED:            return "Element depth exceeded";
    default:                                    return "Unknown error";
    }
}

// ************ PUBLIC ************
#define USE_NEW_TEMPLATES 1

void CTemplateLoader::LoadGameList()
{
#if USE_NEW_TEMPLATES
    const TString kTemplatesDir = "../templates_new/";

    // Read game list
    {
        const TString kGameListPath = kTemplatesDir + "GameList.xml";
        CXMLReader Reader(kGameListPath);
        ASSERT(Reader.IsValid());

        if (Reader.ParamBegin("Games", 0))
        {
            u32 NumGames;
            Reader.SerializeArraySize(NumGames);

            for (u32 GameIdx = 0; GameIdx < NumGames; GameIdx++)
            {
                if (Reader.ParamBegin("Game", 0))
                {
                    EGame Game;
                    TString Name, MasterPath;

                    Reader << SerialParameter("ID", Game, SH_Attribute)
                           << SerialParameter("Name", Name)
                           << SerialParameter("MasterTemplate", MasterPath);

                    CMasterTemplate* pMaster = new CMasterTemplate();
                    pMaster->mGame = Game;
                    pMaster->mGameName = Name;
                    pMaster->mSourceFile = MasterPath;
                    CMasterTemplate::smMasterMap[Game] = pMaster;
                    Reader.ParamEnd();
                }
            }
            Reader.ParamEnd();
        }
    }
    {
        // Read property list
        const TString kPropertyMapPath = kTemplatesDir + "PropertyMap.xml";
        CXMLReader Reader(kPropertyMapPath);
        ASSERT(Reader.IsValid());
        Reader << SerialParameter("PropertyMap", CMasterTemplate::smPropertyNames, SH_HexDisplay);
    }
    {
        // Read master templates
        std::list<CMasterTemplate*> MasterList = CMasterTemplate::MasterList();

        for (auto Iter = MasterList.begin(); Iter != MasterList.end(); Iter++)
        {
            CMasterTemplate* pMaster = *Iter;
            const TString kMasterPath = pMaster->GetGameDirectory(true) + "Game.xml";

            CXMLReader Reader(kMasterPath);
            ASSERT(Reader.IsValid());
            pMaster->Serialize(Reader);
            pMaster->LoadSubTemplates();
        }
    }
#else
    Log::Write("Loading game list");

    // Load Game List XML
    XMLDocument GameListXML;
    OpenXML(mskGameListPath, GameListXML);

    if (GameListXML.Error())
        return;

    // Parse
    XMLElement *pRoot = GameListXML.FirstChildElement("GameList");
    const char *pkGameListVersion = pRoot->Attribute("version");

    if (pkGameListVersion)
        CMasterTemplate::smGameListVersion = TString(pkGameListVersion).ToInt32(10);

    XMLElement *pElem = pRoot->FirstChildElement();

    while (pElem)
    {
        TString NodeName = TString(pElem->Name()).ToLower();

        // Properties
        if (NodeName == "properties")
        {
            TString PropListPath = pElem->GetText();
            XMLDocument PropListXML;
            OpenXML(mskTemplatesDir + PropListPath, PropListXML);

            if (!PropListXML.Error())
                LoadPropertyList(&PropListXML, PropListPath);
        }

        // Games
        else if (NodeName == "game")
        {
            CTemplateLoader Loader(mskTemplatesDir);
            CMasterTemplate *pMaster = Loader.LoadGameInfo(pElem);
            CMasterTemplate::smMasterMap[pMaster->mGame] = pMaster;
        }

        pElem = pElem->NextSiblingElement();
    }
#endif
}

void CTemplateLoader::LoadGameTemplates(EGame Game)
{
    std::list<CMasterTemplate*> MasterList = CMasterTemplate::MasterList();

    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        if (pMaster->Game() == Game && !pMaster->IsLoadedSuccessfully())
        {
            XMLDocument MasterXML;
            OpenXML(mskTemplatesDir + pMaster->mSourceFile, MasterXML);

            if (!MasterXML.Error())
            {
                CTemplateLoader Loader(mskTemplatesDir);
                Loader.mGame = Game;
                Loader.LoadMasterTemplate(&MasterXML, pMaster);
            }

            break;
        }
    }
}

void CTemplateLoader::LoadAllGames()
{
    std::list<CMasterTemplate*> MasterList = CMasterTemplate::MasterList();

    for (auto it = MasterList.begin(); it != MasterList.end(); it++)
    {
        CMasterTemplate *pMaster = *it;

        if (!pMaster->IsLoadedSuccessfully())
        {
            XMLDocument MasterXML;
            OpenXML(mskTemplatesDir + pMaster->mSourceFile, MasterXML);

            if (!MasterXML.Error())
            {
                CTemplateLoader Loader(mskTemplatesDir);
                Loader.mGame = pMaster->Game();
                Loader.LoadMasterTemplate(&MasterXML, pMaster);
            }
        }
    }
}

void CTemplateLoader::LoadPropertyList(XMLDocument *pDoc, const TString& ListName)
{
    XMLElement *pRootElem = pDoc->FirstChildElement("Properties");

    if (!pRootElem)
        Log::Error(ListName + ": There is no root \"Properties\" block element");

    else
    {
        XMLElement *pElem = pRootElem->FirstChildElement("property");

        while (pElem)
        {
            TString ID = pElem->Attribute("ID");
            TString Name = pElem->Attribute("name");

            if (!ID.IsEmpty() && !Name.IsEmpty())
                CMasterTemplate::smPropertyNames[ID.ToInt32()] = Name;

            pElem = pElem->NextSiblingElement();
        }
    }
}

void CTemplateLoader::SaveGameList()
{
    const TString kTemplatesDir = "../templates_new/";
    FileUtil::MakeDirectory( kTemplatesDir );

    // Write game list
    {
        const TString kGameListPath = kTemplatesDir + "GameList.xml";
        CXMLWriter Writer(kGameListPath, "GameList");

        u32 NumGames = CMasterTemplate::smMasterMap.size();
        Writer.ParamBegin("Games", 0);
        Writer.SerializeArraySize(NumGames);

        for (auto Iter = CMasterTemplate::smMasterMap.begin(); Iter != CMasterTemplate::smMasterMap.end(); Iter++)
        {
            struct SGameInfo
            {
                EGame Game;
                TString Name;
                TString MasterPath;

                void Serialize(IArchive& Arc)
                {
                    Arc << SerialParameter("ID", Game, SH_Attribute)
                        << SerialParameter("Name", Name)
                        << SerialParameter("MasterTemplate", MasterPath);
                }
            };

            CMasterTemplate* pMaster = Iter->second;
            SGameInfo Info;
            Info.Game = pMaster->Game();
            Info.Name = pMaster->GameName();
            Info.MasterPath = pMaster->GetGameDirectory() + "Game.xml";
            Writer << SerialParameter("Game", Info);
        }
        Writer.ParamEnd();
    }

    // Write property list
    {
        CXMLWriter Writer(kTemplatesDir + "PropertyMap.xml", "PropertyMap");
        Writer << SerialParameter("PropertyMap", CMasterTemplate::smPropertyNames, SH_HexDisplay);
    }

    // Write master templates
    {
        std::list<CMasterTemplate*> MasterList = CMasterTemplate::MasterList();

        for (auto Iter = MasterList.begin(); Iter != MasterList.end(); Iter++)
        {
            CMasterTemplate* pMasterTemplate = *Iter;
            TString MasterFilePath = pMasterTemplate->GetGameDirectory(true) + "Game.xml";
            FileUtil::MakeDirectory( MasterFilePath.GetFileDirectory() );

            CXMLWriter Writer(MasterFilePath, "Game", 0, pMasterTemplate->Game());
            pMasterTemplate->Serialize(Writer);
            pMasterTemplate->SaveSubTemplates();
        }
    }
}
