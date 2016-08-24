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
    };
    std::vector<SParameter> mParamStack;

    IOutputStream *mpStream;
    bool mOwnsStream;

public:
    CBinaryWriter(const TString& rkFilename, u32 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOwnsStream(true)
    {
        mpStream = new CFileOutStream(rkFilename.ToStdString(), IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());

        mFileVersion = FileVersion;
        mGame = Game;

        mpStream->WriteShort((u16) FileVersion);
        mpStream->WriteShort((u16) skCurrentArchiveVersion);
        GetGameID(Game).Write(*mpStream);
    }

    CBinaryWriter(IOutputStream *pStream, u32 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;

        mFileVersion = FileVersion;
        mGame = Game;

        mpStream->WriteShort((u16) FileVersion);
        mpStream->WriteShort((u16) skCurrentArchiveVersion);
        GetGameID(Game).Write(*mpStream);
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

        mParamStack.push_back( SParameter { mpStream->Tell(), 0 } );

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
        u16 ParamSize = (EndOffset - StartOffset) - 6;

        mpStream->Seek(StartOffset + 4, SEEK_SET);
        mpStream->WriteShort(ParamSize);
        if (rParam.NumSubParams > 0) mpStream->WriteShort(rParam.NumSubParams);
        mpStream->Seek(EndOffset, SEEK_SET);
        mParamStack.pop_back();
    }

    void SerializeContainerSize(u32& rSize)
    {
        // Normally handled by ParamBegin and ParamEnd but we need to do something here to account for zero-sized containers
        if (rSize == 0)
            mpStream->WriteShort(0);
    }

    void SerializeAbstractObjectType(u32& rType)    { mpStream->WriteLong(rType); }

    void SerializePrimitive(bool& rValue)           { mpStream->WriteBool(rValue); }
    void SerializePrimitive(char& rValue)           { mpStream->WriteByte(rValue); }
    void SerializePrimitive(s8& rValue)             { mpStream->WriteByte(rValue); }
    void SerializePrimitive(u8& rValue)             { mpStream->WriteByte(rValue); }
    void SerializePrimitive(s16& rValue)            { mpStream->WriteShort(rValue); }
    void SerializePrimitive(u16& rValue)            { mpStream->WriteShort(rValue); }
    void SerializePrimitive(s32& rValue)            { mpStream->WriteLong(rValue); }
    void SerializePrimitive(u32& rValue)            { mpStream->WriteLong(rValue); }
    void SerializePrimitive(s64& rValue)            { mpStream->WriteLongLong(rValue); }
    void SerializePrimitive(u64& rValue)            { mpStream->WriteLongLong(rValue); }
    void SerializePrimitive(float& rValue)          { mpStream->WriteFloat(rValue); }
    void SerializePrimitive(double& rValue)         { mpStream->WriteDouble(rValue); }
    void SerializePrimitive(TString& rValue)        { mpStream->WriteSizedString(rValue.ToStdString()); }
    void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream); }

    void SerializeHexPrimitive(s8& rValue)          { mpStream->WriteByte(rValue); }
    void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    void SerializeHexPrimitive(s16& rValue)         { mpStream->WriteShort(rValue); }
    void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    void SerializeHexPrimitive(s32& rValue)         { mpStream->WriteLong(rValue); }
    void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    void SerializeHexPrimitive(s64& rValue)         { mpStream->WriteLongLong(rValue); }
    void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBINARYWRITER

