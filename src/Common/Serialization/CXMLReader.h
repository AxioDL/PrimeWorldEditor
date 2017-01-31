#ifndef CXMLREADER
#define CXMLREADER

#include "IArchive.h"
#include <tinyxml2.h>

class CXMLReader : public IArchive
{
    tinyxml2::XMLDocument mDoc;
    tinyxml2::XMLElement *mpCurElem; // Points to the next element being read
    bool mJustEndedParam; // Indicates we just ended a primitive parameter

public:
    CXMLReader(const TString& rkFileName)
        : IArchive(true, false)
        , mJustEndedParam(false)
    {
        // Load XML and set current element to the root element; read version
        mDoc.LoadFile(*rkFileName);
        mpCurElem = mDoc.FirstChildElement();
        ASSERT(mpCurElem != nullptr);

        mArchiveVersion = (u16) TString( mpCurElem->Attribute("ArchiveVer") ).ToInt32(10);
        mFileVersion = (u16) TString( mpCurElem->Attribute("FileVer") ).ToInt32(10);
        const char *pkGameAttr = mpCurElem->Attribute("Game");
        mGame = pkGameAttr ? GetGameForID( CFourCC(pkGameAttr) ) : eUnknownGame;
    }

    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        // Push new parent if needed
        if (!mJustEndedParam)
        {
            tinyxml2::XMLElement *pChild = mpCurElem->FirstChildElement();
            if (!pChild) return false;
            else mpCurElem = pChild;
        }

        // Verify the current element matches the name of the next serialized parameter.
        if ( strcmp(mpCurElem->Name(), pkName) == 0)
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
        return false;
    }

    virtual void ParamEnd()
    {
        if (mJustEndedParam)
            mpCurElem = mpCurElem->Parent()->ToElement();

        tinyxml2::XMLElement *pElem = mpCurElem->NextSiblingElement();
        if (pElem)
            mpCurElem = pElem;

        mJustEndedParam = true;
    }

protected:
    TString ReadParam()
    {
        return TString(mpCurElem->GetText());
    }

public:
    virtual void SerializeContainerSize(u32& rSize, const TString& rkElemName)
    {
        rSize = 0;

        for (tinyxml2::XMLElement *pElem = mpCurElem->FirstChildElement(*rkElemName); pElem; pElem = pElem->NextSiblingElement(*rkElemName))
            rSize++;
    }

    virtual void SerializeAbstractObjectType(u32& rType)
    {
        rType = TString(mpCurElem->Attribute("Type")).ToInt32(10);
    }

    virtual void SerializePrimitive(bool& rValue)        { rValue = (ReadParam() == "true" ? true : false); }
    virtual void SerializePrimitive(char& rValue)        { rValue = ReadParam().Front(); }
    virtual void SerializePrimitive(s8& rValue)          { rValue = (s8) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(u8& rValue)          { rValue = (u8) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(s16& rValue)         { rValue = (s16) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(u16& rValue)         { rValue = (u16) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(s32& rValue)         { rValue = (s32) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(u32& rValue)         { rValue = (u32) ReadParam().ToInt32(10); }
    virtual void SerializePrimitive(s64& rValue)         { rValue = (s64) ReadParam().ToInt64(10); }
    virtual void SerializePrimitive(u64& rValue)         { rValue = (u64) ReadParam().ToInt64(10); }
    virtual void SerializePrimitive(float& rValue)       { rValue = ReadParam().ToFloat(); }
    virtual void SerializePrimitive(double& rValue)      { rValue = (double) ReadParam().ToFloat(); }
    virtual void SerializePrimitive(TString& rValue)     { rValue = ReadParam(); }
    virtual void SerializePrimitive(TWideString& rValue) { rValue = ReadParam().ToUTF16(); }
    virtual void SerializePrimitive(CFourCC& rValue)     { rValue = CFourCC( ReadParam() ); }
    virtual void SerializePrimitive(CAssetID& rValue)    { rValue = CAssetID::FromString( ReadParam() ); }

    virtual void SerializeHexPrimitive(u8& rValue)       { rValue = (u8) ReadParam().ToInt32(16); }
    virtual void SerializeHexPrimitive(u16& rValue)      { rValue = (u16) ReadParam().ToInt32(16); }
    virtual void SerializeHexPrimitive(u32& rValue)      { rValue = (u32) ReadParam().ToInt32(16); }
    virtual void SerializeHexPrimitive(u64& rValue)      { rValue = (u64) ReadParam().ToInt32(16); }
};

#endif // CXMLREADER

