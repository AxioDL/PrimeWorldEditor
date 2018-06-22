#ifndef CTEMPLATELOADER_H
#define CTEMPLATELOADER_H

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include "Core/Resource/Script/IPropertyNew.h"
#include "Core/Resource/Script/Property/CEnumProperty.h"
#include "Core/Resource/Script/Property/CFlagsProperty.h"
#include <tinyxml2.h>

class CTemplateLoader
{
    static const TString mskTemplatesDir;
    static const TString mskGameListPath;

    CMasterTemplate *mpMaster;
    EGame mGame;
    TString mTemplatesDir;
    TString mMasterDir;

    // Constructor
    CTemplateLoader(const TString& rkTemplatesDir)
        : mTemplatesDir(rkTemplatesDir) {}

    // Load Property
    IPropertyNew* LoadProperty(tinyxml2::XMLElement* pElem, CScriptTemplate* pScript, CStructPropertyNew* pParentStruct, const TString& rkTemplateName);
    IPropertyNew* CreateProperty(u32 ID, EPropertyTypeNew Type, const TString& rkName, CScriptTemplate* pScript, CStructPropertyNew* pStruct);

    CStructPropertyNew* LoadStructArchetype(const TString& rkTemplateFileName);
    CEnumProperty* LoadEnumArchetype(const TString& rkTemplateFileName, bool bIsChoice);
    CFlagsProperty* LoadFlagsArchetype(const TString& rkTemplateFileName);

    void LoadProperties(tinyxml2::XMLElement* pPropertiesElem, CScriptTemplate* pScript, CStructPropertyNew* pStruct, const TString& rkTemplateName);
    void LoadEnumerators(tinyxml2::XMLElement* pEnumeratorsElem, CEnumProperty* pEnum, const TString& rkTemplateName);
    void LoadBitFlags(tinyxml2::XMLElement* pFlagsElem, CFlagsProperty* pFlags, const TString& rkTemplateName);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument* pDoc, const TString& rkTemplateName, u32 ObjectID);

    // Load Master
    CMasterTemplate* LoadGameInfo(tinyxml2::XMLNode* pNode);
    void LoadMasterTemplate(tinyxml2::XMLDocument* pDoc, CMasterTemplate* pMaster);

    // Utility
    static void OpenXML(const TString& rkPath, tinyxml2::XMLDocument& rDoc);
    static TString ErrorName(tinyxml2::XMLError Error);

public:
    static void LoadGameList();
    static void LoadGameTemplates(EGame Game);
    static void LoadAllGames();
    static void LoadPropertyList(tinyxml2::XMLDocument* pDoc, const TString& rkListName);
};

#endif // CTEMPLATELOADER_H
