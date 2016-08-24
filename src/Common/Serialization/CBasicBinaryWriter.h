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
    CBasicBinaryWriter(const TString& rkFilename, u32 FileVersion, EGame Game = eUnknownGame, IOUtil::EEndianness = IOUtil::eLittleEndian)
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

    CBasicBinaryWriter(IOutputStream *pStream, u32 FileVersion, EGame Game = eUnknownGame)
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

    ~CBasicBinaryWriter()
    {
        if (mOwnsStream) delete mpStream;
    }

    // Interface
    virtual bool ParamBegin(const char*)    { return true; }
    virtual void ParamEnd()                 { }

    virtual void SerializeContainerSize(u32& rSize)         { SerializePrimitive(rSize); }
    virtual void SerializeAbstractObjectType(u32& rType)    { SerializePrimitive(rType); }
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
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue.Write(*mpStream); }

    virtual void SerializeHexPrimitive(s8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(u8& rValue)          { mpStream->WriteByte(rValue); }
    virtual void SerializeHexPrimitive(s16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(u16& rValue)         { mpStream->WriteShort(rValue); }
    virtual void SerializeHexPrimitive(s32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(u32& rValue)         { mpStream->WriteLong(rValue); }
    virtual void SerializeHexPrimitive(s64& rValue)         { mpStream->WriteLongLong(rValue); }
    virtual void SerializeHexPrimitive(u64& rValue)         { mpStream->WriteLongLong(rValue); }
};

#endif // CBASICBINARYWRITER

