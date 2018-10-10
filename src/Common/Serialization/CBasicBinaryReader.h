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
        : IArchive()
        , mOwnsStream(true)
    {
        mArchiveFlags = AF_Binary | AF_Reader | AF_NoSkipping;
        mpStream = new CFileInStream(rkFilename, IOUtil::eBigEndian);

        if (mpStream->IsValid())
        {
            mMagicValid = (mpStream->ReadLong() == Magic);
            SerializeVersion();
        }
    }

    CBasicBinaryReader(IInputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive()
        , mMagicValid(true)
        , mOwnsStream(false)
    {
        mArchiveFlags = AF_Binary | AF_Reader | AF_NoSkipping;

        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);
    }

    CBasicBinaryReader(void *pData, u32 DataSize, const CSerialVersion& rkVersion, IOUtil::EEndianness Endian = IOUtil::kSystemEndianness)
        : IArchive()
        , mMagicValid(true)
        , mOwnsStream(true)
    {
        mArchiveFlags = AF_Binary | AF_Reader | AF_NoSkipping;
        mpStream = new CMemoryInStream(pData, DataSize, Endian);
        SetVersion(rkVersion);
    }

    ~CBasicBinaryReader()
    {
        if (mOwnsStream) delete mpStream;
    }

    inline bool IsValid() const { return mpStream->IsValid(); }

    // Interface
    virtual bool IsReader() const       { return true; }
    virtual bool IsWriter() const       { return false; }
    virtual bool IsTextFormat() const   { return false; }

    virtual bool ParamBegin(const char*, u32)   { return true; }
    virtual void ParamEnd()                     { }

    virtual bool PreSerializePointer(void*& Pointer, u32 Flags)                 { return ArchiveVersion() >= eArVer_Refactor ? mpStream->ReadBool() : true; }
    virtual void SerializeContainerSize(u32& rSize, const TString&, u32 Flags)  { SerializePrimitive(rSize, Flags); }
    virtual void SerializeBulkData(void* pData, u32 Size, u32 Flags)            { mpStream->ReadBytes(pData, Size); }

    virtual void SerializePrimitive(bool& rValue, u32 Flags)            { rValue = mpStream->ReadBool(); }
    virtual void SerializePrimitive(char& rValue, u32 Flags)            { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(s8& rValue, u32 Flags)              { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(u8& rValue, u32 Flags)              { rValue = mpStream->ReadByte(); }
    virtual void SerializePrimitive(s16& rValue, u32 Flags)             { rValue = mpStream->ReadShort(); }
    virtual void SerializePrimitive(u16& rValue, u32 Flags)             { rValue = mpStream->ReadShort(); }
    virtual void SerializePrimitive(s32& rValue, u32 Flags)             { rValue = mpStream->ReadLong(); }
    virtual void SerializePrimitive(u32& rValue, u32 Flags)             { rValue = mpStream->ReadLong(); }
    virtual void SerializePrimitive(s64& rValue, u32 Flags)             { rValue = mpStream->ReadLongLong(); }
    virtual void SerializePrimitive(u64& rValue, u32 Flags)             { rValue = mpStream->ReadLongLong(); }
    virtual void SerializePrimitive(float& rValue, u32 Flags)           { rValue = mpStream->ReadFloat(); }
    virtual void SerializePrimitive(double& rValue, u32 Flags)          { rValue = mpStream->ReadDouble(); }
    virtual void SerializePrimitive(TString& rValue, u32 Flags)         { rValue = mpStream->ReadSizedString(); }
    virtual void SerializePrimitive(TWideString& rValue, u32 Flags)     { rValue = mpStream->ReadSizedWString(); }
    virtual void SerializePrimitive(CFourCC& rValue, u32 Flags)         { rValue = CFourCC(*mpStream); }
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags)        { rValue = CAssetID(*mpStream, mGame); }
};

#endif // CBASICBINARYREADER

