#ifndef CXMLWRITER
#define CXMLWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"
#include <iostream>
#include <tinyxml2.h>

class CXMLWriter : public IArchive
{
    tinyxml2::XMLDocument mDoc;
    tinyxml2::XMLElement *mpCurElem;
    TString mOutFilename;
    const char* mpAttributeName;
    bool mSaved;

public:
    CXMLWriter(const TString& rkFileName, const TString& rkRootName, u16 FileVersion = 0, EGame Game = EGame::Invalid)
        : IArchive()
        , mOutFilename(rkFileName)
        , mpAttributeName(nullptr)
        , mSaved(false)
    {
        mArchiveFlags = AF_Writer | AF_Text;
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);

        // Create declaration and root node
        tinyxml2::XMLDeclaration *pDecl = mDoc.NewDeclaration();
        mDoc.LinkEndChild(pDecl);

        mpCurElem = mDoc.NewElement(*rkRootName);
        mDoc.LinkEndChild(mpCurElem);

        // Write version data
        SerializeVersion();
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
    virtual bool ParamBegin(const char *pkName, u32 Flags)
    {
        ASSERT(IsValid());
        ASSERT(!mpAttributeName); // Attributes cannot have sub-children

        // Read as attribute if needed
        if (Flags & SH_Attribute)
        {
            mpAttributeName = pkName;
        }
        else
        {
            tinyxml2::XMLElement *pElem = mDoc.NewElement(pkName);
            mpCurElem->LinkEndChild(pElem);
            mpCurElem = pElem;
        }

        return true;
    }

    virtual void ParamEnd()
    {
        if (mpAttributeName)
        {
            mpAttributeName = nullptr;
        }
        else
        {
            // If we didn't save any sub parameters, remove the element.
            tinyxml2::XMLElement* pParent = mpCurElem->Parent()->ToElement();

            if ( mpCurElem->FirstAttribute() == nullptr
                 && mpCurElem->FirstChild() == nullptr
                 && mpCurElem->GetText() == nullptr )
            {
                pParent->DeleteChild(mpCurElem);
            }

            mpCurElem = pParent;
        }
    }

protected:
    void WriteParam(const char *pkValue)
    {
        if (mpAttributeName)
            mpCurElem->SetAttribute(mpAttributeName, pkValue);
        else
            mpCurElem->SetText(pkValue);
    }

public:
    virtual bool PreSerializePointer(void*& Pointer, u32 Flags)
    {
        if (!Pointer)
        {
            mpCurElem->SetText("NULL");
            return false;
        }
        return true;
    }

    virtual void SerializeArraySize(u32& Value)
    {
        // Do nothing. Reader obtains container size from number of child elements
    }

    virtual void SerializePrimitive(bool& rValue, u32 Flags)        { WriteParam(rValue ? "true" : "false"); }
    virtual void SerializePrimitive(char& rValue, u32 Flags)        { WriteParam(*TString(rValue)); }
    virtual void SerializePrimitive(s8& rValue, u32 Flags)          { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u8&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(u8& rValue, u32 Flags)          { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u8&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(s16& rValue, u32 Flags)         { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u16&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(u16& rValue, u32 Flags)         { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u16&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(s32& rValue, u32 Flags)         { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u32&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(u32& rValue, u32 Flags)         { WriteParam( (Flags & SH_HexDisplay) ? *TString::HexString((u32&) rValue, 0) : *TString::FromInt32(rValue, 0, 10) ); }
    virtual void SerializePrimitive(s64& rValue, u32 Flags)         { WriteParam( *TString::FromInt64(rValue, 0, (Flags & SH_HexDisplay) ? 16 : 10) ); }
    virtual void SerializePrimitive(u64& rValue, u32 Flags)         { WriteParam( *TString::FromInt64(rValue, 0, (Flags & SH_HexDisplay) ? 16 : 10) ); }
    virtual void SerializePrimitive(float& rValue, u32 Flags)       { WriteParam( *TString::FromFloat(rValue, 1, true) ); }
    virtual void SerializePrimitive(double& rValue, u32 Flags)      { WriteParam( *TString::FromFloat((float) rValue, 1, true) ); }
    virtual void SerializePrimitive(TString& rValue, u32 Flags)     { WriteParam( *rValue ); }
    virtual void SerializePrimitive(TWideString& rValue, u32 Flags) { WriteParam( *rValue.ToUTF8() ); }
    virtual void SerializePrimitive(CFourCC& rValue, u32 Flags)     { WriteParam( *rValue.ToString() ); }
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags)    { WriteParam( *rValue.ToString( CAssetID::GameIDLength(Game()) ) ); }

    virtual void SerializeBulkData(void* pData, u32 Size, u32 Flags)
    {
        char* pCharData = (char*) pData;
        TString OutString(Size*2);

        for (u32 ByteIdx = 0; ByteIdx < Size; ByteIdx++)
        {
            //@todo: sloooooow. maybe replace with a LUT?
            sprintf(&OutString[ByteIdx*2], "%02X", pCharData[ByteIdx]);
        }

        WriteParam(*OutString);
    }
};

#endif // CXMLWRITER
