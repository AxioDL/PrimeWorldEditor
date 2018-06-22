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
    bool mSaved;

public:
    CXMLWriter(const TString& rkFileName, const TString& rkRootName, u16 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOutFilename(rkFileName)
        , mSaved(false)
    {
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);

        // Create declaration and root node
        tinyxml2::XMLDeclaration *pDecl = mDoc.NewDeclaration();
        mDoc.LinkEndChild(pDecl);

        mpCurElem = mDoc.NewElement(*rkRootName);
        mDoc.LinkEndChild(mpCurElem);

        // Write version data
        mpCurElem->SetAttribute("ArchiveVer", (int) skCurrentArchiveVersion);
        mpCurElem->SetAttribute("FileVer", (int) FileVersion);
        if (Game != eUnknownGame) mpCurElem->SetAttribute("Game", *GetGameID(Game).ToString());
    }

    ~CXMLWriter()
    {
        if (!mSaved)
        {
            bool SaveSuccess = Save();
            ASSERT(SaveSuccess);
        }
    }

    inline bool Save()
    {
        if (mSaved)
        {
            Log::Error("Attempted to save XML twice!");
            return false;
        }

        tinyxml2::XMLError Error = mDoc.SaveFile(*mOutFilename);
        mSaved = true;

        if (Error != tinyxml2::XML_SUCCESS)
        {
            Log::Error("Failed to save XML file: " + mOutFilename);
            return false;
        }
        else
            return true;
    }

    inline bool IsValid() const
    {
        return mpCurElem != nullptr && !mSaved;
    }

    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        ASSERT(IsValid());

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
    virtual void SerializeContainerSize(u32&, const TString&)
    {
        // Reader obtains container size from number of child elements
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
    virtual void SerializePrimitive(TWideString& rValue) { WriteParam(*rValue.ToUTF8()); }
    virtual void SerializePrimitive(CFourCC& rValue)     { WriteParam(*rValue.ToString()); }
    virtual void SerializePrimitive(CAssetID& rValue)    { WriteParam(*rValue.ToString( CAssetID::GameIDLength(Game()) )); }

    virtual void SerializeHexPrimitive(u8& rValue)       { WriteParam(*TString::HexString(rValue, 2)); }
    virtual void SerializeHexPrimitive(u16& rValue)      { WriteParam(*TString::HexString(rValue, 4)); }
    virtual void SerializeHexPrimitive(u32& rValue)      { WriteParam(*TString::HexString(rValue, 8)); }
    virtual void SerializeHexPrimitive(u64& rValue)      { WriteParam(*TString::HexString((u32) rValue, 16)); }

    virtual void BulkSerialize(void* pData, u32 Size)
    {
        char* pCharData = (char*) pData;
        TString OutString(Size*2);

        for (u32 ByteIdx = 0; ByteIdx < Size; ByteIdx++)
            itoa(pCharData[ByteIdx], &OutString[ByteIdx*2], 16);

        WriteParam(*OutString);
    }
};

#endif // CXMLWRITER
