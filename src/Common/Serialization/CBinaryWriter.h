#ifndef CBINARYWRITER
#define CBINARYWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"

class CBinaryWriter : public IArchive
{
    struct SParameter
    {
        u32 Offset;
        u32 NumSubParams;
    };
    std::vector<SParameter> mParamStack;

    IOutputStream *mpStream;
    u32 mMagic;
    bool mOwnsStream;

public:
    CBinaryWriter(const TString& rkFilename, u32 Magic, u16 FileVersion = 0, EGame Game = EGame::Invalid)
        : IArchive()
        , mMagic(Magic)
        , mOwnsStream(true)
    {
        mArchiveFlags = AF_Writer | AF_Binary;
        mpStream = new CFileOutStream(rkFilename, IOUtil::eBigEndian);

        if (mpStream->IsValid())
        {
            mpStream->WriteLong(0); // Magic is written after the rest of the file has been successfully written
            SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        }

        InitParamStack();
        SerializeVersion();
    }

    CBinaryWriter(IOutputStream *pStream, u16 FileVersion = 0, EGame Game = EGame::Invalid)
        : IArchive()
        , mMagic(0)
        , mOwnsStream(false)
    {
        ASSERT(pStream && pStream->IsValid());
        mArchiveFlags = AF_Writer | AF_Binary;
        mpStream = pStream;
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        InitParamStack();
    }

    CBinaryWriter(IOutputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive()
        , mMagic(0)
        , mOwnsStream(false)
    {
        ASSERT(pStream && pStream->IsValid());
        mArchiveFlags = AF_Writer | AF_Binary;
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

        // Write magic and delete stream
        if (mOwnsStream)
        {
            mpStream->GoTo(0);
            mpStream->WriteLong(mMagic);
            delete mpStream;
        }
    }

    inline bool IsValid() const { return mpStream->IsValid(); }

private:
    void InitParamStack()
    {
        mParamStack.reserve(20);
        mpStream->WriteLong(0xFFFFFFFF);
        mpStream->WriteLong(0); // Size filler
        mParamStack.push_back( SParameter { mpStream->Tell(), 0 } );
    }

public:
    // Interface
    virtual bool ParamBegin(const char *pkName, u32 Flags)
    {
        // Update parent param
        mParamStack.back().NumSubParams++;

        if (mParamStack.back().NumSubParams == 1)
            mpStream->WriteLong(-1); // Sub-param count filler

        // Write param metadata
        u32 ParamID = TString(pkName).Hash32();
        mpStream->WriteLong(ParamID);
        mpStream->WriteLong(-1); // Param size filler

        // Add new param to the stack
        mParamStack.push_back( SParameter { mpStream->Tell(), 0 } );

        return true;
    }

    virtual void ParamEnd()
    {
        // Write param size
        SParameter& rParam = mParamStack.back();
        u32 StartOffset = rParam.Offset;
        u32 EndOffset = mpStream->Tell();
        u32 ParamSize = (EndOffset - StartOffset);

        mpStream->GoTo(StartOffset - 4);
        mpStream->WriteLong(ParamSize);

        // Write param child count
        if (rParam.NumSubParams > 0 || mParamStack.size() == 1)
        {
            mpStream->WriteLong(rParam.NumSubParams);
        }

        mpStream->GoTo(EndOffset);
        mParamStack.pop_back();
    }

    virtual bool PreSerializePointer(void*& Pointer, u32 Flags)
    {
        bool ValidPtr = (Pointer != nullptr);
        *this << SerialParameter("PointerValid", ValidPtr);
        return ValidPtr;
    }

    virtual void SerializeContainerSize(u32& rSize, const TString& /*rkElemName*/)
    {
        // Normally handled by ParamBegin and ParamEnd but we need to do something here to account for zero-sized containers
        if (rSize == 0)
            mpStream->WriteLong(0);
    }

    virtual void SerializePrimitive(bool& rValue, u32 Flags)            { mpStream->WriteBool(rValue); }
    virtual void SerializePrimitive(char& rValue, u32 Flags)            { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(s8& rValue, u32 Flags)              { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(u8& rValue, u32 Flags)              { mpStream->WriteByte(rValue); }
    virtual void SerializePrimitive(s16& rValue, u32 Flags)             { mpStream->WriteShort(rValue); }
    virtual void SerializePrimitive(u16& rValue, u32 Flags)             { mpStream->WriteShort(rValue); }
    virtual void SerializePrimitive(s32& rValue, u32 Flags)             { mpStream->WriteLong(rValue); }
    virtual void SerializePrimitive(u32& rValue, u32 Flags)             { mpStream->WriteLong(rValue); }
    virtual void SerializePrimitive(s64& rValue, u32 Flags)             { mpStream->WriteLongLong(rValue); }
    virtual void SerializePrimitive(u64& rValue, u32 Flags)             { mpStream->WriteLongLong(rValue); }
    virtual void SerializePrimitive(float& rValue, u32 Flags)           { mpStream->WriteFloat(rValue); }
    virtual void SerializePrimitive(double& rValue, u32 Flags)          { mpStream->WriteDouble(rValue); }
    virtual void SerializePrimitive(TString& rValue, u32 Flags)         { mpStream->WriteSizedString(rValue); }
    virtual void SerializePrimitive(TWideString& rValue, u32 Flags)     { mpStream->WriteSizedWString(rValue); }
    virtual void SerializePrimitive(CFourCC& rValue, u32 Flags)         { rValue.Write(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags)        { rValue.Write(*mpStream, CAssetID::GameIDLength(Game())); }
    virtual void SerializeBulkData(void* pData, u32 Size, u32 Flags)    { mpStream->WriteBytes(pData, Size); }
};

#endif // CBINARYWRITER

