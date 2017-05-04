#include "CVectorOutStream.h"

CVectorOutStream::CVectorOutStream()
    : mpVector(new std::vector<char>)
    , mOwnsVector(true)
    , mPos(0)
    , mUsed(0)
{
    mDataEndianness = IOUtil::eBigEndian;
}

CVectorOutStream::CVectorOutStream(IOUtil::EEndianness DataEndianness)
    : mpVector(new std::vector<char>)
    , mOwnsVector(true)
    , mPos(0)
    , mUsed(0)
{
    mDataEndianness = DataEndianness;
}

CVectorOutStream::CVectorOutStream(u32 InitialSize, IOUtil::EEndianness DataEndianness)
    : mpVector(new std::vector<char>(InitialSize))
    , mOwnsVector(true)
    , mPos(0)
    , mUsed(0)
{
    mDataEndianness = DataEndianness;
}

CVectorOutStream::CVectorOutStream(std::vector<char> *pVector, IOUtil::EEndianness DataEndianness)
    : mpVector(pVector)
    , mOwnsVector(false)
    , mPos(0)
    , mUsed(0)
{
    mDataEndianness = DataEndianness;
}

CVectorOutStream::~CVectorOutStream()
{
    if (mOwnsVector) delete mpVector;
}

void CVectorOutStream::WriteBytes(const void *pkSrc, u32 Count)
{
    if (!IsValid()) return;

    if ((mPos + Count) > mpVector->size())
        mpVector->resize(mPos + Count);

    memcpy(mpVector->data() + mPos, pkSrc, Count);
    mPos += Count;
    if (mPos > mUsed) mUsed = mPos;
}

bool CVectorOutStream::Seek(s32 Offset, u32 Origin)
{
    if (!IsValid()) return false;

    switch (Origin)
    {
        case SEEK_SET:
            mPos = Offset;
            break;

        case SEEK_CUR:
            mPos += Offset;
            break;

        case SEEK_END:
            mPos = mUsed - Offset;
            break;

        default:
            return false;
    }

    if (mPos < 0)
    {
        mPos = 0;
        return false;
    }

    if (mPos > mUsed)
        mUsed = mPos;

    if (mPos > mpVector->size())
        mpVector->resize(mPos);

    return true;
}

u32 CVectorOutStream::Tell() const
{
    return mPos;
}

bool CVectorOutStream::EoF() const
{
    return false;
}

bool CVectorOutStream::IsValid() const
{
    return true;
}

u32 CVectorOutStream::Size() const
{
    return mUsed;
}

u32 CVectorOutStream::SizeRemaining() const
{
    return mpVector->size() - mPos;
}

void CVectorOutStream::SetVector(std::vector<char> *pVector)
{
    if (mOwnsVector) delete mpVector;
    mpVector = pVector;
    mPos = 0;
    mUsed = 0;
}

void* CVectorOutStream::Data()
{
    return mpVector->data();
}

void* CVectorOutStream::DataAtPosition()
{
    return mpVector->data() + mPos;
}

void CVectorOutStream::Expand(u32 Amount)
{
    mpVector->resize(mpVector->size() + Amount);
}

void CVectorOutStream::Shrink()
{
    mpVector->resize(mUsed);
}

void CVectorOutStream::Reset()
{
    mPos = 0;
    mUsed = 0;
}

void CVectorOutStream::Clear()
{
    mpVector->clear();
    Reset();
}
