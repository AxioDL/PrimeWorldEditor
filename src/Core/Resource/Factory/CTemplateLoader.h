#ifndef CTEMPLATELOADER_H
#define CTEMPLATELOADER_H

#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include "Core/Resource/Script/Property/IProperty.h"
#include "Core/Resource/Script/Property/CEnumProperty.h"
#include "Core/Resource/Script/Property/CFlagsProperty.h"
#include <tinyxml2.h>

class CTemplateLoader
{
    static const TString mskTemplatesDir;
    static const TString mskGameListPath;

    CGameTemplate *mpGameTemplate;
    EGame mGame;
    TString mTemplatesDir;
    TString mGameDir;

    // Constructor
    CTemplateLoader(const TString& rkTemplatesDir)
        : mTemplatesDir(rkTemplatesDir) {}

    // Load Property
    IProperty* LoadProperty(tinyxml2::XMLElement* pElem, CScriptTemplate* pScript, CStructProperty* pParentStruct, const TString& rkTemplateName);
    IProperty* CreateProperty(u32 ID, EPropertyType Type, const TString& rkName, CScriptTemplate* pScript, CStructProperty* pStruct);

    CStructProperty* LoadStructArchetype(const TString& rkTemplateFileName);
    CEnumProperty* LoadEnumArchetype(const TString& rkTemplateFileName, bool bIsChoice);
    CFlagsProperty* LoadFlagsArchetype(const TString& rkTemplateFileName);

    void LoadProperties(tinyxml2::XMLElement* pPropertiesElem, CScriptTemplate* pScript, CStructProperty* pStruct, const TString& rkTemplateName);
    void LoadEnumerators(tinyxml2::XMLElement* pEnumeratorsElem, CEnumProperty* pEnum, const TString& rkTemplateName);
    void LoadBitFlags(tinyxml2::XMLElement* pFlagsElem, CFlagsProperty* pFlags, const TString& rkTemplateName);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument* pDoc, const TString& rkTemplateName, u32 ObjectID);

    // Load Game
    CGameTemplate* LoadGameInfo(tinyxml2::XMLNode* pNode);
    void LoadGameTemplate(tinyxml2::XMLDocument* pDoc, CGameTemplate* pGame);

    // Utility
    static void OpenXML(const TString& rkPath, tinyxml2::XMLDocument& rDoc);
    static TString ErrorName(tinyxml2::XMLError Error);

public:
    static void LoadGameList();
    static void LoadGameTemplates(EGame Game);
    static void LoadAllGames();
    static void LoadPropertyList(tinyxml2::XMLDocument* pDoc, const TString& rkListName);

    static void SaveGameList();
};

#endif // CTEMPLATELOADER_H
