#ifndef CBINARYREADER
#define CBINARYREADER

#include "IArchive.h"
#include "CSerialVersion.h"
#include "Common/CFourCC.h"

class CBinaryReader : public IArchive
{
    struct SBinaryParm
    {
        u32 Offset;
        u32 Size;
        u32 NumChildren;
        u32 ChildIndex;
    };
    std::vector<SBinaryParm> mBinaryParmStack;

    IInputStream *mpStream;
    bool mMagicValid;
    bool mOwnsStream;
    bool mInAttribute;

public:
    CBinaryReader(const TString& rkFilename, u32 Magic)
        : IArchive()
        , mOwnsStream(true)
        , mInAttribute(false)
    {
        mArchiveFlags = AF_Reader | AF_Binary;
        mpStream = new CFileInStream(rkFilename, IOUtil::eBigEndian);

        if (mpStream->IsValid())
        {
            mMagicValid = (mpStream->ReadLong() == Magic);
        }

        InitParamStack();
        SerializeVersion();
    }

    CBinaryReader(IInputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive()
        , mMagicValid(true)
        , mOwnsStream(false)
        , mInAttribute(false)
    {
        ASSERT(pStream && pStream->IsValid());
        mArchiveFlags = AF_Reader | AF_Binary;
        mpStream = pStream;
        SetVersion(rkVersion);

        InitParamStack();
    }

    ~CBinaryReader()
    {
        if (mOwnsStream) delete mpStream;
    }

    inline bool IsValid() const { return mpStream->IsValid() && mMagicValid; }

private:
    void InitParamStack()
    {
        mpStream->Skip(4); // Skip root ID (which is always -1)
        u32 Size = ReadSize();
        u32 Offset = mpStream->Tell();
        u32 NumChildren = ReadSize();
        mBinaryParmStack.push_back( SBinaryParm { Offset, Size, NumChildren, 0 } );
        mBinaryParmStack.reserve(20);
    }

public:
    // Interface
    u32 ReadSize()
    {
        return (mArchiveVersion < eArVer_32BitBinarySize ? (u32) mpStream->ReadShort() : mpStream->ReadLong());
    }

    virtual bool ParamBegin(const char *pkName, u32 Flags)
    {
        // If this is the parent parameter's first child, then read the child count
        if (mBinaryParmStack.back().NumChildren == 0xFFFFFFFF)
        {
            mBinaryParmStack.back().NumChildren = ReadSize();
        }

        // Save current offset
        u32 Offset = mpStream->Tell();
        u32 ParamID = TString(pkName).Hash32();

        // Check the next parameter ID first and check whether it's a match for the current parameter
        if (mBinaryParmStack.back().ChildIndex < mBinaryParmStack.back().NumChildren)
        {
            u32 NextID = mpStream->ReadLong();
            u32 NextSize = ReadSize();

            // Does the next parameter ID match the current one?
            if (NextID == ParamID || (Flags & SH_IgnoreName))
            {
                mBinaryParmStack.push_back( SBinaryParm { mpStream->Tell(), NextSize, 0xFFFFFFFF, 0 } );
                return true;
            }
        }

        // It's not a match - return to the parent parameter's first child and check all children to find a match
        if (!mBinaryParmStack.empty())
        {
            u32 ParentOffset = mBinaryParmStack.back().Offset;
            u32 NumChildren = mBinaryParmStack.back().NumChildren;
            mpStream->GoTo(ParentOffset);

            for (u32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
            {
                u32 ChildID = mpStream->ReadLong();
                u32 ChildSize = ReadSize();

                if (ChildID != ParamID)
                    mpStream->Skip(ChildSize);
                else
                {
                    mBinaryParmStack.back().ChildIndex = ChildIdx;
                    mBinaryParmStack.push_back( SBinaryParm { mpStream->Tell(), ChildSize, 0xFFFFFFFF, 0 } );
                    return true;
                }
            }
        }

        // None of the children were a match - this parameter isn't in the file
        mpStream->GoTo(Offset);
        return false;
    }

    virtual void ParamEnd()
    {
        // Make sure we're at the end of the parameter
        SBinaryParm& rParam = mBinaryParmStack.back();
        u32 EndOffset = rParam.Offset + rParam.Size;
        mpStream->GoTo(EndOffset);
        mBinaryParmStack.pop_back();

        // Increment parent child index
        if (!mBinaryParmStack.empty())
            mBinaryParmStack.back().ChildIndex++;
    }

    virtual bool PreSerializePointer(void*& Pointer, u32 Flags)
    {
        if (ArchiveVersion() >= eArVer_Refactor)
        {
            bool ValidPtr = (Pointer != nullptr);
            *this << SerialParameter("PointerValid", ValidPtr);
            return ValidPtr;
        }
        else
        {
            return true;
        }
    }

    virtual void SerializeContainerSize(u32& rSize, const TString& /*rkElemName*/)
    {
        // Mostly handled by ParamBegin, we just need to return the size correctly so the container can be resized
        rSize = (mArchiveVersion < eArVer_32BitBinarySize ? (u32) mpStream->PeekShort() : mpStream->PeekLong());
    }

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
    virtual void SerializePrimitive(CAssetID& rValue, u32 Flags)        { rValue = CAssetID(*mpStream, Game()); }
    virtual void SerializeBulkData(void* pData, u32 Size, u32 Flags)    { mpStream->ReadBytes(pData, Size); }
};

#endif // CBINARYREADER

