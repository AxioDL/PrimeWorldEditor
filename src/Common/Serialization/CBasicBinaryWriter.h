#ifndef CBASICBINARYWRITER
#define CBASICBINARYWRITER

#include "IArchive.h"
#include "Common/CFourCC.h"
#include <FileIO/IOutputStream.h>

// This is a basic binary reader that doesn't do any checks on parameter names.
// This is the fastest serializer, but it relies entirely on parameter order so
// changes to file structure will break old versions of the file. Use carefully!
class CBasicBinaryWriter : public IArchive
{
    IOutputStream *mpStream;
    bool mOwnsStream;

public:
    CBasicBinaryWriter(const TString& rkFilename, u16 FileVersion, EGame Game = eUnknownGame, IOUtil::EEndianness = IOUtil::eLittleEndian)
        : IArchive(false, true)
        , mOwnsStream(true)
    {
        mpStream = new CFileOutStream(rkFilename.ToStdString(), IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());
        SetVersion(skCurrentArchiveVersion, FileVersion, Game);
        GetVersionInfo().Write(*mpStream);
    }

    CBasicBinaryWriter(IOutputStream *pStream, u16 FileVersion, EGame Game = eUnknownGame)
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
        if (mOwnsStream) delete mpStream;
    }

    // Interface
    virtual bool ParamBegin(const char*)    { return true; }
    virtual void ParamEnd()                 { }

    virtual void SerializeContainerSize(u32& rSize)         { mpStream->WriteLong(rSize); }
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
    virtual void SerializePrimitive(TString& rValue)        { mpStream->WriteSizedString(rValue.ToStdString()); }
    virtual void SerializePrimitive(TWideString& rValue)    { mpStream->WriteSizedWideString(rValue.ToStdString()); }
    virtual void SerializePrimitive(CFourCC& rValue)        { rValue.Write(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream); }

    virtual void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBASICBINARYWRITER

