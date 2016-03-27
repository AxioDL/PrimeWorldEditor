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

CVectorOutStream::CVectorOutStream(unsigned long InitialSize, IOUtil::EEndianness DataEndianness)
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

void CVectorOutStream::WriteBytes(void *pSrc, unsigned long Count)
{
    if (!IsValid()) return;

    if ((mPos + Count) > mpVector->size())
        mpVector->resize(mPos + Count);

    memcpy(mpVector->data() + mPos, pSrc, Count);
    mPos += Count;
    if (mPos > mUsed) mUsed = mPos;
}

bool CVectorOutStream::Seek(long Offset, long Origin)
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

    if (mPos > (signed long) mpVector->size())
        mpVector->resize(mPos);

    return true;
}

long CVectorOutStream::Tell() const
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

long CVectorOutStream::Size() const
{
    return mUsed;
}

long CVectorOutStream::SizeRemaining() const
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

void CVectorOutStream::Expand(unsigned long Amount)
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
