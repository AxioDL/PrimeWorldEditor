#ifndef CBINARYREADER
#define CBINARYREADER

#include "IArchive.h"
#include "CSerialVersion.h"
#include "Common/CFourCC.h"

class CBinaryReader : public IArchive
{
    struct SParameter
    {
        u32 Offset;
        u16 Size;
        u16 NumChildren;
        u16 ChildIndex;
        bool Abstract;
    };
    std::vector<SParameter> mParamStack;

    IInputStream *mpStream;
    bool mOwnsStream;

public:
    CBinaryReader(const TString& rkFilename)
        : IArchive(true, false)
        , mOwnsStream(true)
    {
        mpStream = new CFileInStream(rkFilename, IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());

        CSerialVersion Version(*mpStream);
        SetVersion(Version);

        InitParamStack();
    }

    CBinaryReader(IInputStream *pStream, const CSerialVersion& rkVersion)
        : IArchive(true, false)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;
        SetVersion(rkVersion);

        InitParamStack();
    }

    ~CBinaryReader()
    {
        if (mOwnsStream) delete mpStream;
    }

private:
    void InitParamStack()
    {
        mpStream->Skip(4); // Skip root ID (which is always -1)
        u16 Size = mpStream->ReadShort();
        u32 Offset = mpStream->Tell();
        u16 NumChildren = mpStream->ReadShort();
        mParamStack.push_back( SParameter { Offset, Size, NumChildren, 0, false } );
        mParamStack.reserve(20);
    }

public:
    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        // If this is the parent parameter's first child, then read the child count
        if (mParamStack.back().NumChildren == 0xFFFF)
        {
            mParamStack.back().NumChildren = mpStream->ReadShort();
        }

        // Save current offset
        u32 Offset = mpStream->Tell();
        u32 ParamID = TString(pkName).Hash32();

        // Check the next parameter ID first and check whether it's a match for the current parameter
        if (mParamStack.back().ChildIndex < mParamStack.back().NumChildren)
        {
            u32 NextID = mpStream->ReadLong();
            u16 NextSize = mpStream->ReadShort();

            // Does the next parameter ID match the current one?
            if (NextID == ParamID)
            {
                mParamStack.push_back( SParameter { Offset, NextSize, 0xFFFF, 0, false } );
                return true;
            }
        }

        // It's not a match - return to the parent parameter's first child and check all children to find a match
        if (!mParamStack.empty())
        {
            bool ParentAbstract = mParamStack.back().Abstract;
            u32 ParentOffset = mParamStack.back().Offset;
            u16 NumChildren = mParamStack.back().NumChildren;
            mpStream->GoTo(ParentOffset + (ParentAbstract ? 0xC : 0x8));

            for (u32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
            {
                u32 ChildID = mpStream->ReadLong();
                u16 ChildSize = mpStream->ReadShort();

                if (ChildID != ParamID)
                    mpStream->Skip(ChildSize);
                else
                {
                    mParamStack.back().ChildIndex = (u16) ChildIdx;
                    mParamStack.push_back( SParameter { mpStream->Tell() - 6, ChildSize, 0xFFFF, 0, false } );
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
        SParameter& rParam = mParamStack.back();
        u32 EndOffset = rParam.Offset + rParam.Size + 6;
        mpStream->GoTo(EndOffset);
        mParamStack.pop_back();

        // Increment parent child index
        if (!mParamStack.empty())
            mParamStack.back().ChildIndex++;
    }

    virtual void SerializeContainerSize(u32& rSize, const TString& /*rkElemName*/)
    {
        // Mostly handled by ParamBegin, we just need to return the size correctly so the container can be resized
        rSize = (u32) mpStream->PeekShort();
    }

    virtual void SerializeAbstractObjectType(u32& rType)
    {
        // Mark current parameter as abstract so we can account for the object type in the filestream
        rType = mpStream->ReadLong();
        mParamStack.back().Abstract = true;
    }

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
    virtual void SerializePrimitive(CAssetID& rValue)       { rValue = CAssetID(*mpStream, Game()); }

    virtual void SerializeHexPrimitive(u8& rValue)          { rValue = mpStream->ReadByte(); }
    virtual void SerializeHexPrimitive(u16& rValue)         { rValue = mpStream->ReadShort(); }
    virtual void SerializeHexPrimitive(u32& rValue)         { rValue = mpStream->ReadLong(); }
    virtual void SerializeHexPrimitive(u64& rValue)         { rValue = mpStream->ReadLongLong(); }
};

#endif // CBINARYREADER

