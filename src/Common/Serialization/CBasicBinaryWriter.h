#ifndef CBASICBINARYWRITER
#define CBASICBINARYWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"
#include "Common/FileIO/IOutputStream.h"

// This is a basic binary reader that doesn't do any checks on parameter names.
// This is the fastest serializer, but it relies entirely on parameter order so
// changes to file structure will break old versions of the file. Use carefully!
class CBasicBinaryWriter : public IArchive
{
    IOutputStream *mpStream;
    u32 mMagic;
    bool mOwnsStream;

public:
    CBasicBinaryWriter(const TString& rkFilename, u32 Magic, u16 FileVersion, EGame Game)
        : IArchive(false, true)
        , mMagic(Magic)
        , mOwnsStream(true)
    {
        mpStream = new CFileOutStream(rkFilename, IOUtil::eBigEndian);

        if (mpStream->IsValid())
        {
            mpStream->WriteLong(0); // Magic is written after the rest of the file is successfully saved
            SetVersion(skCurrentArchiveVersion, FileVersion, Game);
            GetVersionInfo().Write(*mpStream);
        }
    }

    CBasicBinaryWriter(IOutputStream *pStream, u16 FileVersion, EGame Game)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
    }

    CBasicBinaryWriter(IOutputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive(false, true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);
    }

    ~CBasicBinaryWriter()
    {
        // Write magic and delete stream
        if (mOwnsStream)
        {
            mpStream->GoTo(0);
            mpStream->WriteLong(mMagic);
            delete mpStream;
        }
    }

    inline bool IsValid() const { return mpStream->IsValid(); }

    // Interface
    virtual bool ParamBegin(const char*)    { return true; }
    virtual void ParamEnd()                 { }

    virtual void SerializeContainerSize(u32& rSize, const TString&) { mpStream->WriteLong(rSize); }
    virtual void SerializeAbstractObjectType(u32& rType)    { mpStream->WriteLong(rType); }
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
    virtual void SerializePrimitive(TWideString& rValue)    { mpStream->WriteSizedWString(rValue); }
    virtual void SerializePrimitive(CFourCC& rValue)        { rValue.Write(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream, CAssetID::GameIDLength(Game())); }

    virtual void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBASICBINARYWRITER

