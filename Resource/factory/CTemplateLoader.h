#ifndef CTEMPLATELOADER_H
#define CTEMPLATELOADER_H

#include "../script/CMasterTemplate.h"
#include "../script/CScriptTemplate.h"
#include <tinyxml2.h>

class CTemplateLoader
{
    CMasterTemplate *mpMaster;
    std::string mTemplatesDir;
    std::string mMasterDir;

    // Constructor
    CTemplateLoader(const std::string& templatesDir) : mTemplatesDir(templatesDir) {}

    // Load Property
    void LoadStructProperties(tinyxml2::XMLElement *pElem, CStructTemplate *pTemp, const std::string& templateName);
    CPropertyTemplate* LoadPropertyTemplate(tinyxml2::XMLElement *pElem, const std::string& templateName);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument *pDoc, const std::string& templateName, u32 objectID);

    // Load Master
    void LoadMasterTemplate(tinyxml2::XMLDocument *pDoc);
    void LoadPropertyList(tinyxml2::XMLDocument *pDoc, const std::string& listName);
    CMasterTemplate* LoadGame(tinyxml2::XMLNode *pNode);

public:
    static void LoadGameList();
};

#endif // CTEMPLATELOADER_H
