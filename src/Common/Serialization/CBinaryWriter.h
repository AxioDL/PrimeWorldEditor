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
        mpStream = new CFileOutStream(rkFilename, IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());

        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        GetVersionInfo().Write(*mpStream);
        InitParamStack();
    }

    CBinaryWriter(IOutputStream *pStream, u16 FileVersion, EGame Game = eUnknownGame)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        InitParamStack();
    }

    CBinaryWriter(IOutputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);
        InitParamStack();
    }

    ~CBinaryWriter()
    {
        // Ensure all params have been finished
        ASSERT(mParamStack.size() == 1);

        // Finish root param
        ParamEnd();

        // Delete stream
        if (mOwnsStream)
            delete mpStream;
    }

private:
    void InitParamStack()
    {
        mParamStack.reserve(20);
        mpStream->WriteLong(0xFFFFFFFF);
        mpStream->WriteShort(0); // Size filler
        mParamStack.push_back( SParameter { mpStream->Tell(), 0, false } );
    }

public:
    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        // Update parent param
        mParamStack.back().NumSubParams++;

        if (mParamStack.back().NumSubParams == 1)
            mpStream->WriteShort(-1); // Sub-param count filler

        // Write param metadata
        u32 ParamID = TString(pkName).Hash32();
        mpStream->WriteLong(ParamID);
        mpStream->WriteShort((u16) 0xFFFF); // Param size filler

        // Add new param to the stack
        mParamStack.push_back( SParameter { mpStream->Tell(), 0, false } );

        return true;
    }

    virtual void ParamEnd()
    {
        // Write param size
        SParameter& rParam = mParamStack.back();
        u32 StartOffset = rParam.Offset;
        u32 EndOffset = mpStream->Tell();
        u16 ParamSize = (u16) (EndOffset - StartOffset);

        mpStream->GoTo(StartOffset - 2);
        mpStream->WriteShort(ParamSize);

        // Write param child count
        if (rParam.NumSubParams > 0 || mParamStack.size() == 1)
        {
            if (rParam.Abstract) mpStream->Skip(4);
            mpStream->WriteShort(rParam.NumSubParams);
        }

        mpStream->GoTo(EndOffset);
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
    virtual void SerializePrimitive(TString& rValue)        { mpStream->WriteSizedString(rValue); }
    virtual void SerializePrimitive(TWideString& rValue)    { mpStream->WriteSizedWideString(rValue); }
    virtual void SerializePrimitive(CFourCC& rValue)        { rValue.Write(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream, CAssetID::GameIDLength(Game())); }

    virtual void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBINARYWRITER

