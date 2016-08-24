#ifndef CBINARYREADER
#define CBINARYREADER

#include "IArchive.h"
#include "Common/CFourCC.h"

class CBinaryReader : public IArchive
{
    struct SParameter
    {
        u32 Offset;
        u16 Size;
        bool HasChildren;
    };
    std::vector<SParameter> mParamStack;

    IInputStream *mpStream;
    bool mOwnsStream;

public:
    CBinaryReader(const TString& rkFilename)
        : IArchive(true, false)
        , mOwnsStream(true)
    {
        mpStream = new CFileInStream(rkFilename.ToStdString(), IOUtil::eBigEndian);
        ASSERT(mpStream->IsValid());

        mFileVersion = mpStream->ReadShort();
        mArchiveVersion = mpStream->ReadShort();
        mGame = GetGameForID( CFourCC(*mpStream) );

        mParamStack.reserve(20);
    }

    CBinaryReader(IInputStream *pStream)
        : IArchive(true, false)
        , mOwnsStream(false)
    {
        ASSERT(pStream->IsValid());
        mpStream = pStream;

        mFileVersion = mpStream->ReadShort();
        mArchiveVersion = mpStream->ReadShort();
        mGame = GetGameForID( CFourCC(*mpStream) );

        mParamStack.reserve(20);
    }

    ~CBinaryReader()
    {
        if (mOwnsStream) delete mpStream;
    }

    // Interface
    virtual bool ParamBegin(const char *pkName)
    {
        // If this is the parent parameter's first child, then skip the child count
        if (!mParamStack.empty() && !mParamStack.back().HasChildren)
        {
            mpStream->Seek(0x2, SEEK_CUR);
            mParamStack.back().HasChildren = true;
        }

        // Check the next parameter ID first and check whether it's a match for the current parameter
        u32 ParamID = TString(pkName).Hash32();
        u32 Offset = mpStream->Tell();
        u32 NextID = mpStream->ReadLong();
        u16 NextSize = mpStream->ReadShort();

        // Does the next parameter ID match the current one?
        if (NextID == ParamID)
        {
            mParamStack.push_back( SParameter { Offset, NextSize, false } );
            return true;
        }

        // It's not a match - return to the parent parameter's first child and check all children to find a match
        if (!mParamStack.empty())
        {
            u32 ParentOffset = mParamStack.back().Offset;
            mpStream->Seek(ParentOffset, SEEK_SET);
            mpStream->Seek(0x6, SEEK_CUR);
            u16 NumChildren = mpStream->ReadShort();

            for (u32 iChild = 0; iChild < NumChildren; iChild++)
            {
                u32 ChildID = mpStream->ReadLong();
                u16 ChildSize = mpStream->ReadShort();

                if (ChildID != ParamID)
                    mpStream->Seek(ChildSize, SEEK_CUR);
                else
                {
                    mParamStack.push_back( SParameter { mpStream->Tell() - 6, NextSize, false } );
                    return true;
                }
            }
        }

        // None of the children were a match - this parameter isn't in the file
        mpStream->Seek(Offset, SEEK_SET);
        return false;
    }

    virtual void ParamEnd()
    {
        // Make sure we're at the end of the parameter
        SParameter& rParam = mParamStack.back();
        u32 EndOffset = rParam.Offset + rParam.Size + 6;
        mpStream->Seek(EndOffset, SEEK_SET);
        mParamStack.pop_back();
    }

    void SerializeContainerSize(u32& rSize)
    {
        // Mostly handled by ParamBegin, we just need to return the size correctly so the container can be resized
        rSize = (u32) mpStream->PeekShort();
    }

    void SerializeAbstractObjectType(u32& rType)    { rType = mpStream->ReadLong(); }

    void SerializePrimitive(bool& rValue)           { rValue = mpStream->ReadBool(); }
    void SerializePrimitive(char& rValue)           { rValue = mpStream->ReadByte(); }
    void SerializePrimitive(s8& rValue)             { rValue = mpStream->ReadByte(); }
    void SerializePrimitive(u8& rValue)             { rValue = mpStream->ReadByte(); }
    void SerializePrimitive(s16& rValue)            { rValue = mpStream->ReadShort(); }
    void SerializePrimitive(u16& rValue)            { rValue = mpStream->ReadShort(); }
    void SerializePrimitive(s32& rValue)            { rValue = mpStream->ReadLong(); }
    void SerializePrimitive(u32& rValue)            { rValue = mpStream->ReadLong(); }
    void SerializePrimitive(s64& rValue)            { rValue = mpStream->ReadLongLong(); }
    void SerializePrimitive(u64& rValue)            { rValue = mpStream->ReadLongLong(); }
    void SerializePrimitive(float& rValue)          { rValue = mpStream->ReadFloat(); }
    void SerializePrimitive(double& rValue)         { rValue = mpStream->ReadDouble(); }
    void SerializePrimitive(TString& rValue)        { rValue = mpStream->ReadSizedString(); }
    void SerializePrimitive(CAssetID& rValue)       { rValue = CAssetID(*mpStream, Game()); }

    void SerializeHexPrimitive(s8& rValue)          { rValue = mpStream->ReadByte(); }
    void SerializeHexPrimitive(u8& rValue)          { rValue = mpStream->ReadByte(); }
    void SerializeHexPrimitive(s16& rValue)         { rValue = mpStream->ReadShort(); }
    void SerializeHexPrimitive(u16& rValue)         { rValue = mpStream->ReadShort(); }
    void SerializeHexPrimitive(s32& rValue)         { rValue = mpStream->ReadLong(); }
    void SerializeHexPrimitive(u32& rValue)         { rValue = mpStream->ReadLong(); }
    void SerializeHexPrimitive(s64& rValue)         { rValue = mpStream->ReadLongLong(); }
    void SerializeHexPrimitive(u64& rValue)         { rValue = mpStream->ReadLongLong(); }
};

#endif // CBINARYREADER

