#ifndef CTEMPLATEWRITER_H
#define CTEMPLATEWRITER_H

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <tinyxml2.h>

class CTemplateWriter
{
    CTemplateWriter();

public:
    static void SaveAllTemplates();
    static void SaveGameTemplates(CMasterTemplate *pMaster, const TString& rkDir);
    static void SavePropertyList(const TString& rkDir);
    static void SaveScriptTemplate(CScriptTemplate *pTemp, const TString& rkDir);
    static void SaveStructTemplate(CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& rkDir);
    static void SaveEnumTemplate(CEnumTemplate *pTemp, const TString& rkDir);
    static void SaveBitfieldTemplate(CBitfieldTemplate *pTemp, const TString& rkDir);
    static void SaveProperties(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CStructTemplate *pTemp, CMasterTemplate *pMaster, const TString& rkDir);
    static void SaveEnumerators(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CEnumTemplate *pTemp);
    static void SaveBitFlags(tinyxml2::XMLDocument *pDoc, tinyxml2::XMLElement *pParent, CBitfieldTemplate *pTemp);
};

#endif // CTEMPLATEWRITER_H
