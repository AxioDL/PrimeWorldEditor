#include "CTemplateLoader.h"
#include "CWorldLoader.h"
#include "../script/EAttribType.h"
#include <Core/Log.h>

// ************ PROPERTY ************
CPropertyTemplate* CTemplateLoader::LoadPropertyTemplate(tinyxml2::XMLElement *pElem, const std::string& TemplateName)
{
    const char *pElemName = pElem->Name();

    // Load multi-property struct
    if (strcmp(pElemName, "struct") == 0)
    {
        CStructTemplate *pStruct = LoadStructTemplate(pElem, TemplateName);

        if (pStruct)
            pStruct->mIsSingleProperty = false;

        return pStruct;
    }

    else if (strcmp(pElemName, "property") == 0)
    {
        // Get name, type, and ID
        std::string Name;
        EPropertyType Type;
        u32 ID;
        GetPropertyInfo(pElem, Name, Type, ID);

        // Error check
        if (Type == eInvalidProperty)
        {
            const char *pType = pElem->Attribute("type");

            if (pType)
                Log::Error("Invalid property type in " + TemplateName + " template: " + pType);
            else
                Log::Error("Property " + Name + " in " + TemplateName + " template has no type");
        }

        // Load single-property struct
        if (Type == eStructProperty)
        {
            CStructTemplate *pStruct = LoadStructTemplate(pElem, TemplateName);
            pStruct->mIsSingleProperty = true;
            return pStruct;
        }

        // Load file property
        else if (Type == eFileProperty)
        {
            // Fetch file extension
            CFileTemplate *pFile = nullptr;
            const char *pExt = pElem->Attribute("ext");

            if (pExt)
                pFile = new CFileTemplate(Name, ID, StringUtil::Tokenize(pExt, ","));

            else
            {
                CFileTemplate *pSrc = (CFileTemplate*) mpMaster->GetProperty(ID);

                if (pSrc)
                    pFile = new CFileTemplate(Name, ID, pSrc->Extensions());
            }

            // Check if extensions are valid
            if (!pFile)
            {
                Log::Error("File property " + Name + " in " + TemplateName + " template has no extension");
                return nullptr;
            }

            else
                return pFile;
        }

        // Load regular property
        else
        {
            CPropertyTemplate *pProperty = new CPropertyTemplate(Type, Name, ID);
            return pProperty;
        }
    }

    return nullptr;
}

CStructTemplate* CTemplateLoader::LoadStructTemplate(tinyxml2::XMLElement *pElem, const std::string& TemplateName)
{
    CStructTemplate *pStruct = new CStructTemplate();

    // Get name, type, and ID
    GetPropertyInfo(pElem, pStruct->mPropName, pStruct->mPropType, pStruct->mPropID);
    const char *pTemp = pElem->Attribute("template");
    if (!pTemp) pTemp = pElem->Attribute("target");

    // Get source template from the master property list, if it exists
    CStructTemplate *pSrc = (CStructTemplate*) mpMaster->GetProperty(pStruct->mPropID);

    // "IsSingleProperty" means, does the struct contain multiple properties, each with separate IDs
    // or does the entire struct as a whole count as just one property?
    if (pSrc)
        pStruct->mIsSingleProperty = pSrc->IsSingleProperty();
    else
        pStruct->mIsSingleProperty = (strcmp(pElem->Name(), "property") == 0);

    // Read struct children. Priority is [Embedded -> Template -> Master].
    // Embedded
    if (!pElem->NoChildren())
    {
        // Get count
        const char *pCount = pElem->Attribute("count");

        if (pCount)
            pStruct->mPropertyCount = std::stoul(pCount);

        // Parse sub-elements
        tinyxml2::XMLElement *pChild = pElem->FirstChildElement();

        while (pChild)
        {
            CPropertyTemplate *pProp = LoadPropertyTemplate(pChild, TemplateName);

            if (pProp)
                pStruct->mProperties.push_back(pProp);

            pChild = pChild->NextSiblingElement();
        }

    }

    // Template
    else if (pTemp)
    {
        // Get handle for XML
        std::string TempPath = mMasterDir + pTemp;

        tinyxml2::XMLDocument TempXML;
        TempXML.LoadFile(TempPath.c_str());

        if (TempXML.Error())
            Log::Error("Couldn't open struct template: " + TempPath);

        else
        {
            tinyxml2::XMLElement *pVersionElem = TempXML.FirstChildElement()->FirstChildElement("version");
            tinyxml2::XMLElement *pPropertiesElem = TempXML.FirstChildElement()->FirstChildElement("properties");

            if (!pVersionElem) Log::Error("Struct template has no version element: " + TempPath);
            if (!pPropertiesElem) Log::Error("Struct template has no properties element: " + TempPath);

            if (pVersionElem && pPropertiesElem)
            {
                // Get version number
                u32 VersionNumber = std::stoul(pVersionElem->GetText());

                // Get property count
                const char *pCount = pPropertiesElem->Attribute("count");

                if (pCount)
                    pStruct->mPropertyCount = std::stoul(pCount);

                // Parse properties
                tinyxml2::XMLElement *pPropElem = pPropertiesElem->FirstChildElement();

                while (pPropElem)
                {
                    if (!pPropElem) break;

                    CPropertyTemplate *pProp = LoadPropertyTemplate(pPropElem, TemplateName);

                    if (pProp)
                        pStruct->mProperties.push_back(pProp);

                    pPropElem = pPropElem->NextSiblingElement();
                }
            }
        }
    }

    // Master
    else if (pSrc)
    {
        pStruct->mPropertyCount = pSrc->TemplateCount();

        for (u32 p = 0; p < pSrc->Count(); p++)
            pStruct->mProperties.push_back(pSrc->PropertyByIndex(p));
    }

    // If it's none of these things, then it probably has no children because it's a property list entry
    return pStruct;
}

void CTemplateLoader::GetPropertyInfo(tinyxml2::XMLElement *pElem, std::string& Name, EPropertyType& Type, u32& ID)
{
    const char *pNameStr = pElem->Attribute("name");
    const char *pTypeStr = pElem->Attribute("type");
    const char *pIDStr   = pElem->Attribute("ID");
    bool IsBaseStruct = (strcmp(pElem->Name(), "properties") == 0);

    // Fetch source template, if available
    CPropertyTemplate *pSrcTmp;

    if (pIDStr)
    {
        ID = std::stoul(pIDStr, 0, 16);
        pSrcTmp = mpMaster->GetProperty(ID);
    }
    else
    {
        ID = 0xFFFFFFFF;
        pSrcTmp = nullptr;
    }

    // Get name
    if (pNameStr)
        Name = pNameStr;
    else if (pSrcTmp)
        Name = pSrcTmp->Name();
    else if (IsBaseStruct)
        Name = "Base";
    else
        Name = "";

    // Get type
    if (strcmp(pElem->Name(), "struct") == 0)
        Type = eStructProperty;
    else if (IsBaseStruct)
        Type = eStructProperty;
    else if (pTypeStr)
        Type = PropStringToPropEnum(pTypeStr);
    else if (pSrcTmp)
        Type = pSrcTmp->Type();
    else
        Type = eInvalidProperty;
}

// ************ SCRIPT OBJECT ************
CScriptTemplate* CTemplateLoader::LoadScriptTemplate(tinyxml2::XMLDocument *pDoc, const std::string& TemplateName, u32 ObjectID)
{
    tinyxml2::XMLElement *pBaseElement = pDoc->FirstChildElement();

    CScriptTemplate *pScript = new CScriptTemplate(mpMaster);
    pScript->mObjectID = ObjectID;
    pScript->mTemplateName = pBaseElement->Name();

    // Properties?
    tinyxml2::XMLElement *pProperties = pBaseElement->FirstChildElement("properties");
    if (pProperties)
    {
        pScript->mpBaseStruct = LoadStructTemplate(pBaseElement->FirstChildElement("properties"), TemplateName);
        pScript->mpBaseStruct->SetName(pScript->mTemplateName);
    }

    // Attribs?
    tinyxml2::XMLElement *pAttributes = pBaseElement->FirstChildElement("attributes");
    if (pAttributes) LoadScriptAttribs(pAttributes, pScript);

    return pScript;
}

void CTemplateLoader::LoadScriptAttribs(tinyxml2::XMLElement *pElem, CScriptTemplate *pScript)
{
    // Parsing attribs
    tinyxml2::XMLElement *pAttrib = pElem->FirstChildElement("attrib");
    while (pAttrib)
    {
        CAttribTemplate Attrib;
        Attrib.ExtraSettings = -1;

        const char *pType = pAttrib->Attribute("type");
        if (!pType)
            Log::Error("An attrib in " + pScript->TemplateName() + " template has no type set");

        else
        {
            // Initialize attrib template values
            Attrib.AttribType = AttribStringToAttribEnum(pType);

            if (Attrib.AttribType == eInvalidAttrib)
                Log::Error("An attrib in " + pScript->TemplateName() + " template has an invalid type: " + pType);

            else
            {
                bool NoError = ParseAttribExtra(pAttrib, Attrib, pScript->TemplateName());

                if (NoError)
                {
                    Attrib.AttribTarget = pAttrib->Attribute("target");
                    CPropertyTemplate *pTargetProp = nullptr;

                    if (Attrib.ResFile.empty())
                    {
                        pTargetProp = pScript->mpBaseStruct->PropertyByName(Attrib.AttribTarget); // Ensure target is valid if it points to a property

                        if (!pTargetProp)
                           Log::Error("An attrib in " + pScript->TemplateName() + " template of type " + pType + " has an invalid target: " + Attrib.AttribTarget);
                    }

                    if ((pTargetProp) || (!Attrib.ResFile.empty()))
                        pScript->mAttribs.push_back(Attrib);
                }
            }
        }

        pAttrib = pAttrib->NextSiblingElement("attrib");
    }
}

bool CTemplateLoader::ParseAttribExtra(tinyxml2::XMLElement *pElem, CAttribTemplate& Attrib, const std::string& TemplateName)
{
    // This function is for parsing extra tags that some attribs have, such as "source" for models or "forcenode" for animsets

    // AnimSet
    if (Attrib.Type() == eAnimSetAttrib)
    {
        // Check res source
        const char *pSource = pElem->Attribute("source");

        if ((pSource) && (strcmp(pSource, "file") == 0))
        {
            const char *pFileName = pElem->Attribute("target");
            if (pFileName)
                Attrib.ResFile = std::string("../resources/") + pFileName;
            else
            {
                Log::Error("An attrib in " + TemplateName + " template of type animset has an invalid target: \"" + pFileName + "\"");
                return false;
            }
        }

        // Check forcenode
        const char *pForceNode = pElem->Attribute("forcenode");
        if (pForceNode)
        {
            if (!StringUtil::IsHexString(pForceNode))
            {
                Log::Error("An animset attrib in " + TemplateName + " has an invalid \"forcenode\" setting: \"" + pForceNode + "\"");
                return false;
            }
            else
                Attrib.ExtraSettings = std::stoul(pForceNode);
        }
    }

    // Model
    if (Attrib.Type() == eModelAttrib)
    {
        // Check res source
        const char *pSource = pElem->Attribute("source");
        if ((pSource) && (strcmp(pSource, "file") == 0))
        {
            const char *pFileName = pElem->Attribute("target");
            if (pFileName)
                Attrib.ResFile = std::string("../resources/") + pFileName;
            else
            {
                Log::Error("An attrib in " + TemplateName + " template of type model has an invalid target: \"" + pFileName + "\"");
                return false;
            }
        }
    }

    // Volume
    if (Attrib.Type() == eVolumeAttrib)
    {
        const char *pShape = pElem->Attribute("shape");

        if (pShape)
        {
            if (strcmp(pShape, "Box") == 0)
                Attrib.ExtraSettings = 0;
            else if (strcmp(pShape, "OrientedBox") == 0)
                Attrib.ExtraSettings = 1;
            else if (strcmp(pShape, "Sphere") == 0)
                Attrib.ExtraSettings = 2;
            else
            {
                Log::Error("Volume attrib in " + TemplateName + " template has an invalid shape: " + pShape);
                return false;
            }
        }
        else
        {
            Log::Error("Volume attrib in " + TemplateName + " template has no shape attribute");
            return false;
        }
    }

    return true;
}

// ************ MASTER ************
void CTemplateLoader::LoadMasterTemplate(tinyxml2::XMLDocument *pDoc)
{
    tinyxml2::XMLNode *pNode = pDoc->FirstChild()->NextSibling()->FirstChild();

    while (pNode)
    {
        tinyxml2::XMLElement *pElem = pNode->ToElement();

        // Version
        if (strcmp(pElem->Name(), "version") == 0)
        {
            u32 Version = std::stoul(pElem->GetText());
            mpMaster->mVersion = Version;
        }

        // Properties
        else if (strcmp(pElem->Name(), "properties") == 0)
        {
            std::string PropListPath = mMasterDir + pElem->GetText();

            tinyxml2::XMLDocument PropListXML;
            PropListXML.LoadFile(PropListPath.c_str());

            if (PropListXML.Error())
                Log::Error("Couldn't open property list: " + PropListPath);

            else
                LoadPropertyList(&PropListXML, PropListPath);
        }

        // Objects
        else if (strcmp(pElem->Name(), "objects") == 0)
        {
            // Iterate categories
            tinyxml2::XMLElement *pCat = pElem->FirstChildElement("category");

            while (pCat)
            {
                CTemplateCategory Cat(pCat->Attribute("name"));
                tinyxml2::XMLElement *pObj = pCat->FirstChildElement("object");

                while (pObj)
                {
                    // ID can either be a hex number or an ASCII fourCC
                    std::string StrID = pObj->Attribute("ID");
                    u32 ID;

                    if (StringUtil::IsHexString(StrID, true))
                        ID = std::stoul(StrID, 0, 16);
                    else
                        ID = CFourCC(StrID).ToLong();

                    // Load up the object
                    std::string TemplateName = pObj->Attribute("template");
                    std::string TemplatePath = mMasterDir + TemplateName;

                    tinyxml2::XMLDocument ObjectXML;
                    ObjectXML.LoadFile(TemplatePath.c_str());

                    if (ObjectXML.Error())
                        Log::Error("Couldn't open script template: " + TemplatePath);

                    else
                    {
                        CScriptTemplate *pTemp = LoadScriptTemplate(&ObjectXML, TemplateName, ID);

                        if (pTemp)
                        {
                            mpMaster->mTemplates[ID] = pTemp;
                            Cat.AddTemplate(pTemp);
                        }
                    }

                    pObj = pObj->NextSiblingElement("object");
                }

                Cat.Sort();
                mpMaster->mCategories.push_back(Cat);
                pCat = pCat->NextSiblingElement("category");
            }
        }

        // States
        else if (strcmp(pElem->Name(), "states") == 0)
        {
            tinyxml2::XMLElement *pState = pElem->FirstChildElement("state");

            while (pState)
            {
                std::string StrID = pState->Attribute("ID");
                u32 StateID;

                if (StringUtil::IsHexString(StrID, true))
                    StateID = std::stoul(StrID, 0, 16);
                else
                    StateID = CFourCC(StrID).ToLong();

                std::string StateName = pState->Attribute("name");
                mpMaster->mStates[StateID] = StateName;
                pState = pState->NextSiblingElement("state");
            }
        }

        // Messages
        else if (strcmp(pElem->Name(), "messages") == 0)
        {
            tinyxml2::XMLElement *pMessage = pElem->FirstChildElement("message");

            while (pMessage)
            {
                std::string StrID = pMessage->Attribute("ID");
                u32 MessageID;

                if (StringUtil::IsHexString(StrID, true))
                    MessageID = std::stoul(StrID, 0, 16);
                else
                    MessageID = CFourCC(StrID).ToLong();

                std::string MessageName = pMessage->Attribute("name");
                mpMaster->mMessages[MessageID] = MessageName;
                pMessage = pMessage->NextSiblingElement("message");
            }
        }

        pNode = pNode->NextSibling();
    }
}

void CTemplateLoader::LoadPropertyList(tinyxml2::XMLDocument *pDoc, const std::string& ListName)
{
    tinyxml2::XMLElement *pElem = pDoc->FirstChildElement()->FirstChildElement();

    while (pElem)
    {
        CPropertyTemplate *pProp = LoadPropertyTemplate(pElem, ListName);

        if (pProp)
            mpMaster->mPropertyList[pProp->PropertyID()] = pProp;

        pElem = pElem->NextSiblingElement();
    }
}

CMasterTemplate* CTemplateLoader::LoadGame(tinyxml2::XMLNode *pNode)
{
    tinyxml2::XMLElement *pGameElem = pNode->FirstChildElement();
    mpMaster = new CMasterTemplate();

    // Parse game parameters
    while (pGameElem)
    {
        if (strcmp(pGameElem->Name(), "name") == 0)
            mpMaster->mGameName = pGameElem->GetText();

        else if (strcmp(pGameElem->Name(), "mlvl") == 0)
        {
            u32 VersionNum = std::stoul(pGameElem->GetText(), 0, 16);
            mpMaster->mGame = CWorldLoader::GetFormatVersion(VersionNum);
        }

        else if (strcmp(pGameElem->Name(), "master") == 0)
        {
            std::string MasterPath = mTemplatesDir + pGameElem->GetText();
            mMasterDir = StringUtil::GetFileDirectory(MasterPath);

            tinyxml2::XMLDocument MasterXML;
            MasterXML.LoadFile(MasterPath.c_str());

            if (MasterXML.Error())
                Log::Error("Couldn't open master template at " + MasterPath + " - error " + std::to_string(MasterXML.ErrorID()));
            else
                LoadMasterTemplate(&MasterXML);
        }
        pGameElem = pGameElem->NextSiblingElement();
    }

    mpMaster->mFullyLoaded = true;
    return mpMaster;
}

// ************ PUBLIC ************
void CTemplateLoader::LoadGameList()
{
    static const std::string TemplatesDir = "../templates/";
    static const std::string GameListPath = TemplatesDir + "GameList.xml";
    Log::Write("Loading game list");

    // Load Game List XML
    tinyxml2::XMLDocument GameListXML;
    GameListXML.LoadFile(GameListPath.c_str());

    if (GameListXML.Error())
    {
        Log::Error("Couldn't open game list at " + GameListPath + " - error " + std::to_string(GameListXML.ErrorID()));
        return;
    }

    // Parse
    tinyxml2::XMLNode *pNode = GameListXML.FirstChild()->NextSibling()->FirstChild();

    while (pNode)
    {
        tinyxml2::XMLElement *pElement = pNode->ToElement();

        // Game List version number
        if (strcmp(pElement->Name(), "version") == 0)
        {
            u32 VersionNum = std::stoul(pElement->GetText());
            CMasterTemplate::smGameListVersion = VersionNum;
        }

        // Games
        else if (strcmp(pElement->Name(), "game") == 0)
        {
            CTemplateLoader Loader(TemplatesDir);
            CMasterTemplate *pMaster = Loader.LoadGame(pNode);

            if (!pMaster->IsLoadedSuccessfully())
            {
                Log::Error("Master template for " + pMaster->mGameName + " couldn't be loaded");
                delete pMaster;
            }

            else
                CMasterTemplate::smMasterMap[pMaster->mGame] = pMaster;
        }

        pNode = pNode->NextSibling();
    }
}
