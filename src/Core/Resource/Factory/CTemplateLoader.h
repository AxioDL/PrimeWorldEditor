#ifndef CTEMPLATELOADER_H
#define CTEMPLATELOADER_H

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <tinyxml2.h>

class CTemplateLoader
{
    CMasterTemplate *mpMaster;
    TString mTemplatesDir;
    TString mMasterDir;

    // Constructor
    CTemplateLoader(const TString& templatesDir) : mTemplatesDir(templatesDir) {}

    // Load Property
    void LoadBitFlags(tinyxml2::XMLElement *pElem, CBitfieldTemplate *pTemp, const TString& templateName);
    void LoadEnumerators(tinyxml2::XMLElement *pElem, CEnumTemplate *pTemp, const TString& templateName);
    void LoadStructProperties(tinyxml2::XMLElement *pElem, CStructTemplate *pTemp, const TString& templateName);
    CPropertyTemplate* LoadPropertyTemplate(tinyxml2::XMLElement *pElem, const TString& templateName);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument *pDoc, const TString& templateName, u32 objectID);

    // Load Master
    void LoadMasterTemplate(tinyxml2::XMLDocument *pDoc);
    void LoadPropertyList(tinyxml2::XMLDocument *pDoc, const TString& listName);
    CMasterTemplate* LoadGame(tinyxml2::XMLNode *pNode);

public:
    static void LoadGameList();
};

#endif // CTEMPLATELOADER_H
