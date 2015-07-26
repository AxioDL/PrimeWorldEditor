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
    CTemplateLoader(const std::string& TemplatesDir) : mTemplatesDir(TemplatesDir) {}

    // Load Property
    CPropertyTemplate* LoadPropertyTemplate(tinyxml2::XMLElement *pElem, const std::string& TemplateName);
    CStructTemplate* LoadStructTemplate(tinyxml2::XMLElement *pElem, const std::string& TemplateName);
    void GetPropertyInfo(tinyxml2::XMLElement *pElem, std::string& Name, EPropertyType& Type, u32& ID);

    // Load Script Object
    CScriptTemplate* LoadScriptTemplate(tinyxml2::XMLDocument *pDoc, const std::string& TemplateName, u32 ObjectID);
    void LoadScriptAttribs(tinyxml2::XMLElement *pElem, CScriptTemplate *pScript);
    bool ParseAttribExtra(tinyxml2::XMLElement *pElem, CAttribTemplate& Attrib, const std::string& TemplateName);

    // Load Master
    void LoadMasterTemplate(tinyxml2::XMLDocument *pDoc);
    void LoadPropertyList(tinyxml2::XMLDocument *pDoc, const std::string& ListName);
    CMasterTemplate* LoadGame(tinyxml2::XMLNode *pNode);

public:
    static void LoadGameList();
};

#endif // CTEMPLATELOADER_H
