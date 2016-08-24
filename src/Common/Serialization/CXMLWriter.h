#ifndef CXMLWRITER
#define CXMLWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"
#include <iostream>
#include <tinyxml2.h>

class CXMLWriter : public IArchive
{
    tinyxml2::XMLDocument mDoc;
    TString mOutFilename;
    tinyxml2::XMLElement *mpCurElem;

public:
    CXMLWriter(const TString& rkFileName, const TString& rkRootName, u32 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOutFilename(rkFileName)
    {
        mFileVersion = FileVersion;
        mGame = Game;

        // Create declaration and root node
        tinyxml2::XMLDeclaration *pDecl = mDoc.NewDeclaration();
        mDoc.LinkEndChild(pDecl);

        mpCurElem = mDoc.NewElement(*rkRootName);
        mDoc.LinkEndChild(mpCurElem);

        // Write version data
        mpCurElem->SetAttribute("FileVer", (int) FileVersion);
        mpCurElem->SetAttribute("ArchiveVer", (int) skCurrentArchiveVersion);
        if (Game != eUnknownGame) mpCurElem->SetAttribute("Game", *GetGameID(Game).ToString());
    }

    ~CXMLWriter()
    {
        mDoc.SaveFile(*mOutFilename);
    }

    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        tinyxml2::XMLElement *pElem = mDoc.NewElement(pkName);
        mpCurElem->LinkEndChild(pElem);
        mpCurElem = pElem;
        return true;
    }

    virtual void ParamEnd()
    {
        mpCurElem = mpCurElem->Parent()->ToElement();
    }

protected:
    void WriteParam(const char *pkValue)
    {
        mpCurElem->SetText(pkValue);
    }

public:
    virtual void SerializeContainerSize(u32& rSize)
    {
        mpCurElem->SetAttribute("Size", (unsigned int) rSize);
    }

    virtual void SerializeAbstractObjectType(u32& rType)
    {
        mpCurElem->SetAttribute("Type", (unsigned int) rType);
    }

    virtual void SerializePrimitive(bool& rValue)        { WriteParam(rValue ? "true" : "false"); }
    virtual void SerializePrimitive(char& rValue)        { WriteParam(*TString(rValue)); }
    virtual void SerializePrimitive(s8& rValue)          { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(u8& rValue)          { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(s16& rValue)         { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(u16& rValue)         { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(s32& rValue)         { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(u32& rValue)         { WriteParam(*TString::FromInt32(rValue, 0, 10)); }
    virtual void SerializePrimitive(s64& rValue)         { WriteParam(*TString::FromInt64(rValue, 0, 10)); }
    virtual void SerializePrimitive(u64& rValue)         { WriteParam(*TString::FromInt64(rValue, 0, 10)); }
    virtual void SerializePrimitive(float& rValue)       { WriteParam(*TString::FromFloat(rValue)); }
    virtual void SerializePrimitive(double& rValue)      { WriteParam(*TString::FromFloat((float) rValue)); }
    virtual void SerializePrimitive(TString& rValue)     { WriteParam(*rValue); }
    virtual void SerializePrimitive(CAssetID& rValue)    { WriteParam(*rValue.ToString()); }

    virtual void SerializeHexPrimitive(s8& rValue)       { WriteParam(*TString::HexString((u8) rValue, 2)); }
    virtual void SerializeHexPrimitive(u8& rValue)       { WriteParam(*TString::HexString(rValue, 2)); }
    virtual void SerializeHexPrimitive(s16& rValue)      { WriteParam(*TString::HexString((u16) rValue, 4)); }
    virtual void SerializeHexPrimitive(u16& rValue)      { WriteParam(*TString::HexString(rValue, 4)); }
    virtual void SerializeHexPrimitive(s32& rValue)      { WriteParam(*TString::HexString((u32) rValue, 8)); }
    virtual void SerializeHexPrimitive(u32& rValue)      { WriteParam(*TString::HexString(rValue, 8)); }
    virtual void SerializeHexPrimitive(s64& rValue)      { WriteParam(*TString::HexString((u32) rValue, 16)); }
    virtual void SerializeHexPrimitive(u64& rValue)      { WriteParam(*TString::HexString((u32) rValue, 16)); }
};

#endif // CXMLWRITER
