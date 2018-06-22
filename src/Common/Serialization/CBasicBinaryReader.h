#ifndef CBASICBINARYREADER
#define CBASICBINARYREADER

#include "IArchive.h"
#include "CSerialVersion.h"
#include "Common/CFourCC.h"
#include "Common/FileIO/IInputStream.h"

// This is a basic binary reader that doesn't do any checks on parameter names.
// This is the fastest serializer, but it relies entirely on parameter order so
// changes to file structure will break old versions of the file. Use carefully!
class CBasicBinaryReader : public IArchive
{
    IInputStream *mpStream;
    bool mMagicValid;
    bool mOwnsStream;

public:
    CBasicBinaryReader(const TString& rkFilename, u32 Magic)
        : IArchive(true, false)
        , mOwnsStream(true)
    {
        mpStream = new CFileInStream(rkFilename, IOUtil::eBigEndian);

        if (mpStream->IsValid())
        {
            mMagicValid = (mpStream->ReadLong() == Magic);
            CSerialVersion Version(*mpStream);
            SetVersion(Version);
        }
    }

    CBasicBinaryReader(IInputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive(true, false)
        , mMagicValid(true)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);
    }

    CBasicBinaryReader(void *pData, u32 DataSize, const CSerialVersion& rkVersion, IOUtil::EEndianness Endian = IOUtil::kSystemEndianness)
        : IArchive(true, false)
        , mMagicValid(true)
        , mOwnsStream(true)
    {
        mpStream = new CMemoryInStream(pData, DataSize, Endian);
        SetVersion(rkVersion);
    }

    ~CBasicBinaryReader()
    {
        if (mOwnsStream) delete mpStream;
    }

    inline bool IsValid() const { return mpStream->IsValid(); }

    // Interface
    virtual bool ParamBegin(const char*)    { return true; }
    virtual void ParamEnd()                 { }

    virtual void SerializeContainerSize(u32& rSize, const TString&) { SerializePrimitive(rSize); }
    virtual void SerializeAbstractObjectType(u32& rType)    { SerializePrimitive(rType); }
    virtual void SerializePrimitive(bool& rValue)           { rValue = mpStream->ReadBool(); }
    virtual void SerializePrimitive(char& rValue)           { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(s8& rValue)             { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(u8& rValue)             { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(s16& rValue)            { rValue = mpStream->ReadShort(); }
    virtual void SerializePrimitive(u16& rValue)            { rValue = mpStream->ReadShort(); }
    virtual void SerializePrimitive(s32& rValue)            { rValue = mpStream->ReadLong(); }
    virtual void SerializePrimitive(u32& rValue)            { rValue = mpStream->ReadLong(); }
    virtual void SerializePrimitive(s64& rValue)            { rValue = mpStream->ReadLongLong(); }
    virtual void SerializePrimitive(u64& rValue)            { rValue = mpStream->ReadLongLong(); }
    virtual void SerializePrimitive(float& rValue)          { rValue = mpStream->ReadFloat(); }
    virtual void SerializePrimitive(double& rValue)         { rValue = mpStream->ReadDouble(); }
    virtual void SerializePrimitive(TString& rValue)        { rValue = mpStream->ReadSizedString(); }
    virtual void SerializePrimitive(TWideString& rValue)    { rValue = mpStream->ReadSizedWString(); }
    virtual void SerializePrimitive(CFourCC& rValue)        { rValue = CFourCC(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue = CAssetID(*mpStream, mGame); }

    virtual void SerializeHexPrimitive(u8& rValue)          { rValue = mpStream->ReadByte(); }
    virtual void SerializeHexPrimitive(u16& rValue)         { rValue = mpStream->ReadShort(); }
    virtual void SerializeHexPrimitive(u32& rValue)         { rValue = mpStream->ReadLong(); }
    virtual void SerializeHexPrimitive(u64& rValue)         { rValue = mpStream->ReadLongLong(); }

    virtual void BulkSerialize(void* pData, u32 Size)       { mpStream->ReadBytes(pData, Size); }
};

#endif // CBASICBINARYREADER

