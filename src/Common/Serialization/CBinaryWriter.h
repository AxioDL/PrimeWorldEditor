#ifndef CBINARYWRITER
#define CBINARYWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"

class CBinaryWriter : public IArchive
{
    struct SParameter
    {
        u32 Offset;
        u16 NumSubParams;
        bool Abstract;
    };
    std::vector<SParameter> mParamStack;

    IOutputStream *mpStream;
    bool mOwnsStream;

public:
    CBinaryWriter(const TString& rkFilename, u16 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOwnsStream(true)
    {
        mpStream = new CFileOutStream(rkFilename.ToStdString(), IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());

        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        GetVersionInfo().Write(*mpStream);
    }

    CBinaryWriter(IOutputStream *pStream, u16 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
    }

    CBinaryWriter(IOutputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);
    }

    ~CBinaryWriter()
    {
        if (mOwnsStream) delete mpStream;
    }

    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        if (!mParamStack.empty())
        {
            mParamStack.back().NumSubParams++;

            if (mParamStack.back().NumSubParams == 1)
                mpStream->WriteShort(-1); // Sub-param count filler
        }

        mParamStack.push_back( SParameter { mpStream->Tell(), 0, false } );

        u32 ParamID = TString(pkName).Hash32();
        mpStream->WriteLong(ParamID);
        mpStream->WriteShort((u16) 0xFFFF); // Param size filler
        return true;
    }

    virtual void ParamEnd()
    {
        SParameter& rParam = mParamStack.back();
        u32 StartOffset = rParam.Offset;
        u32 EndOffset = mpStream->Tell();
        u16 ParamSize = (u16) (EndOffset - StartOffset) - 6;

        mpStream->Seek(StartOffset + 4, SEEK_SET);
        mpStream->WriteShort(ParamSize);

        if (rParam.NumSubParams > 0)
        {
            if (rParam.Abstract) mpStream->Seek(4, SEEK_CUR);
            mpStream->WriteShort(rParam.NumSubParams);
        }

        mpStream->Seek(EndOffset, SEEK_SET);
        mParamStack.pop_back();
    }

    virtual void SerializeContainerSize(u32& rSize, const TString& /*rkElemName*/)
    {
        // Normally handled by ParamBegin and ParamEnd but we need to do something here to account for zero-sized containers
        if (rSize == 0)
            mpStream->WriteShort(0);
    }

    virtual void SerializeAbstractObjectType(u32& rType)
    {
        // Mark this parameter as abstract so we can account for the object type in the filestream
        mpStream->WriteLong(rType);
        mParamStack.back().Abstract = true;
    }

    virtual void SerializePrimitive(bool& rValue)           { mpStream->WriteBool(rValue); }
    virtual void SerializePrimitive(char& rValue)           { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(s8& rValue)             { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(u8& rValue)             { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(s16& rValue)            { mpStream->WriteShort(rValue); }
    virtual void SerializePrimitive(u16& rValue)            { mpStream->WriteShort(rValue); }
    virtual void SerializePrimitive(s32& rValue)            { mpStream->WriteLong(rValue); }
    virtual void SerializePrimitive(u32& rValue)            { mpStream->WriteLong(rValue); }
    virtual void SerializePrimitive(s64& rValue)            { mpStream->WriteLongLong(rValue); }
    virtual void SerializePrimitive(u64& rValue)            { mpStream->WriteLongLong(rValue); }
    virtual void SerializePrimitive(float& rValue)          { mpStream->WriteFloat(rValue); }
    virtual void SerializePrimitive(double& rValue)         { mpStream->WriteDouble(rValue); }
    virtual void SerializePrimitive(TString& rValue)        { mpStream->WriteSizedString(rValue.ToStdString()); }
    virtual void SerializePrimitive(TWideString& rValue)    { mpStream->WriteSizedWideString(rValue.ToStdString()); }
    virtual void SerializePrimitive(CFourCC& rValue)        { rValue.Write(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream); }

    virtual void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBINARYWRITER

