#ifndef CTEMPLATEWRITER_H
#define CTEMPLATEWRITER_H

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <tinyxml2.h>

class CTemplateWriter
{
    CTemplateWriter();
    static TString smTemplatesDir;

public:
    static void SavePropertyTemplate(IPropertyTemplate *pTemp);
    static void SaveAllTemplates();
    static void SaveGameTemplates(CMasterTemplate *pMaster);
    static void SavePropertyList();
    static void SaveScriptTemplate(CScriptTemplate *pTemp);
    static void SaveStructTemplate(CStructTemplate *pTemp);
    static void SaveEnumTemplate(CEnumTemplate *pTemp);
    static void SaveBitfieldTemplate(CBitfieldTemplate *pTemp);
    static void SaveProperties(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CStructTemplate *pTemp);
    static void SavePropertyOverrides(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CStructTemplate *pStruct, CStructTemplate *pOriginal);
    static void SaveEnumerators(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CEnumTemplate *pTemp);
    static void SaveBitFlags(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CBitfieldTemplate *pTemp);
};

#endif // CTEMPLATEWRITER_H
