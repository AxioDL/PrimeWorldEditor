#ifndef CXMLREADER
#define CXMLREADER

#include "IArchive.h"
#include <tinyxml2.h>

class CXMLReader : public IArchive
{
    tinyxml2::XMLDocument mDoc;
    tinyxml2::XMLElement *mpCurElem; // Points to the next element being read
    const char* mpAttribute; // Name of the parameter we are reading from an attribute
    bool mJustEndedParam; // Indicates we just ended a primitive parameter

public:
    CXMLReader(const TString& rkFileName)
        : IArchive()
        , mpAttribute(nullptr)
        , mJustEndedParam(false)
    {
        mArchiveFlags = AF_Reader | AF_Text;

        // Load XML and set current element to the root element; read version
        mDoc.LoadFile(*rkFileName);
        mpCurElem = mDoc.FirstChildElement();

        if (mpCurElem != nullptr)
        {
            SerializeVersion();
        }
        else
        {
            Log::Error("Failed to open XML for read: " + rkFileName);
        }
    }

    inline bool IsValid() const
    {
        return mpCurElem != nullptr;
    }

    // Interface
    virtual bool IsReader() const       { return true; }
    virtual bool IsWriter() const       { return false; }
    virtual bool IsTextFormat() const   { return true; }

    virtual bool ParamBegin(const char *pkName, u32 Flags)
    {
        ASSERT(IsValid());
        ASSERT(!mpAttribute); // Attributes cannot have sub-children

        // Store as an attribute if requested
        if (Flags & SH_Attribute)
        {
            mpAttribute = mpCurElem->Attribute(pkName);
            return mpAttribute != nullptr;
        }

        // Push new parent if needed
        if (!mJustEndedParam)
        {
            tinyxml2::XMLElement *pChild = mpCurElem->FirstChildElement();
            if (!pChild) return false;
            else mpCurElem = pChild;
        }

        // Verify the current element matches the name of the next serialized parameter.
        if ( (Flags & SH_IgnoreName) || strcmp(mpCurElem->Name(), pkName) == 0 )
        {
            mJustEndedParam = false;
            return true;
        }

        // It didn't match, so we'll try to find a sibling element that does match.
        // Iterate over all sibling elements - if we find a match we will continue
        // reading from that point on. Otherwise we can't load this parameter.
        tinyxml2::XMLElement *pSearchElem = mpCurElem->Parent()->FirstChildElement();

        while (pSearchElem)
        {
            if ( strcmp(pSearchElem->Name(), pkName) == 0 )
            {
                mpCurElem = pSearchElem;
                mJustEndedParam = false;
                return true;
            }

            pSearchElem = pSearchElem->NextSiblingElement();
        }

        // We couldn't find a matching element, so we can't load this parameter.
        // If we pushed a parent earlier, undo it.
        if (!mJustEndedParam)
            mpCurElem = mpCurElem->Parent()->ToElement();

        return false;
    }

    virtual void ParamEnd()
    {
        if (mpAttribute)
            mpAttribute = nullptr;

        else
        {
            if (mJustEndedParam)
                mpCurElem = mpCurElem->Parent()->ToElement();

            tinyxml2::XMLElement *pElem = mpCurElem->NextSiblingElement();
            if (pElem)
                mpCurElem = pElem;

            mJustEndedParam = true;
        }
    }

protected:
    TString ReadParam()
    {
        return TString( mpAttribute ? mpAttribute : mpCurElem->GetText() );
    }

public:
    virtual void SerializeArraySize(u32& Value)
    {
        Value = 0;

        for (tinyxml2::XMLElement *pElem = mpCurElem->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
            Value++;
    }

    virtual bool PreSerializePointer(void*& InPointer, u32 Flags)
    {
        return mpCurElem->GetText() == nullptr || strcmp(mpCurElem->GetText(), "NULL") != 0;
    }

    virtual void SerializePrimitive(bool& rValue, u32 Flags)        { rValue = (ReadParam() == "true" ? true : false); }
    virtual void SerializePrimitive(char& rValue, u32 Flags)        { rValue = ReadParam().Front(); }
    virtual void SerializePrimitive(s8& rValue, u32 Flags)          { rValue = (s8) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(u8& rValue, u32 Flags)          { rValue = (u8) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(s16& rValue, u32 Flags)         { rValue = (s16) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(u16& rValue, u32 Flags)         { rValue = (u16) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(s32& rValue, u32 Flags)         { rValue = (s32) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(u32& rValue, u32 Flags)         { rValue = (u32) ReadParam().ToInt32( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(s64& rValue, u32 Flags)         { rValue = (s64) ReadParam().ToInt64( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(u64& rValue, u32 Flags)         { rValue = (u64) ReadParam().ToInt64( (Flags & SH_HexDisplay) ? 16 : 10 ); }
    virtual void SerializePrimitive(float& rValue, u32 Flags)       { rValue = ReadParam().ToFloat(); }
    virtual void SerializePrimitive(double& rValue, u32 Flags)      { rValue = (double) ReadParam().ToFloat(); }
    virtual void SerializePrimitive(TString& rValue, u32 Flags)     { rValue = ReadParam(); }
    virtual void SerializePrimitive(TWideString& rValue, u32 Flags) { rValue = ReadParam().ToUTF16(); }
    virtual void SerializePrimitive(CFourCC& rValue, u32 Flags)     { rValue = CFourCC( ReadParam() ); }
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags)    { rValue = CAssetID::FromString( ReadParam() ); }

    virtual void SerializeBulkData(void* pData, u32 Size, u32 Flags)
    {
        char* pCharData = (char*) pData;
        TString StringData = ReadParam();
        ASSERT(StringData.Size() == Size*2);

        for (u32 ByteIdx = 0; ByteIdx < Size; ByteIdx++)
        {
            *pCharData = (char) StringData.SubString(ByteIdx*2, 2).ToInt32(16);
            pCharData++;
        }
    }
};

#endif // CXMLREADER

