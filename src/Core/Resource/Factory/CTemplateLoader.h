#ifndef CTEMPLATELOADER_H
#define CTEMPLATELOADER_H

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <tinyxml2.h>

class CTemplateLoader
{
    static const TString mskTemplatesDir;
    static const TString mskGameListPath;

    CMasterTemplate *mpMaster;
    TString mTemplatesDir;
    TString mMasterDir;

    // Constructor
    CTemplateLoader(const TString& rkTemplatesDir)
        : mTemplatesDir(rkTemplatesDir) {}

    // Load Property
    IPropertyTemplate* LoadProperty(tinyxml2::XMLElement *pElem, CStructTemplate *pParentStruct, const TString& rkTemplateName);
    IPropertyTemplate* CreateProperty(u32 ID, EPropertyType Type, const TString& rkName, CStructTemplate *pStruct);

    void LoadStructTemplate(const TString& rkTemplateFileName, CStructTemplate *pStruct);
    void LoadEnumTemplate(const TString& rkTemplateFileName, CEnumTemplate *pEnum);
    void LoadBitfieldTemplate(const TString& rkTemplateFileName, CBitfieldTemplate *pBitfield);

    void LoadProperties(tinyxml2::XMLElement *pPropertiesElem, CStructTemplate *pStruct, const TString& rkTemplateName);
    void LoadEnumerators(tinyxml2::XMLElement *pEnumeratorsElem, CEnumTemplate *pEnum, const TString& rkTemplateName);
    void LoadBitFlags(tinyxml2::XMLElement *pFlagsElem, CBitfieldTemplate *pBitfield, const TString& rkTemplateName);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument *pDoc, const TString& rkTemplateName, u32 ObjectID);

    // Load Master
    CMasterTemplate* LoadGameInfo(tinyxml2::XMLNode *pNode);
    void LoadMasterTemplate(tinyxml2::XMLDocument *pDoc, CMasterTemplate *pMaster);
    void LoadPropertyList(tinyxml2::XMLDocument *pDoc, const TString& rkListName);

    // Utility
    static void OpenXML(const TString& rkPath, tinyxml2::XMLDocument& rDoc);
    static TString ErrorName(tinyxml2::XMLError Error);

public:
    static void LoadGameList();
    static void LoadGameTemplates(EGame Game);
};

#endif // CTEMPLATELOADER_H
