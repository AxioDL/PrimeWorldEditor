#include "CVectorOutStream.h"

CVectorOutStream::CVectorOutStream()
{
    mDataEndianness = IOUtil::eBigEndian;
    mPos = 0;
    mUsed = 0;
}

CVectorOutStream::CVectorOutStream(IOUtil::EEndianness DataEndianness)
{
    mDataEndianness = DataEndianness;
    mPos = 0;
    mUsed = 0;
}

CVectorOutStream::CVectorOutStream(unsigned long InitialSize, IOUtil::EEndianness DataEndianness)
{
    mDataEndianness = DataEndianness;
    mVector.resize(InitialSize);
    mPos = 0;
    mUsed = 0;
}

CVectorOutStream::~CVectorOutStream()
{
}

void CVectorOutStream::WriteBytes(void *pSrc, unsigned long Count)
{
    if (!IsValid()) return;

    if ((mPos + Count) > mVector.size())
        mVector.resize(mPos + Count);

    memcpy(mVector.data() + mPos, pSrc, Count);
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

    if (mPos < 0) {
        mPos = 0;
        return false;
    }

    if (mPos > mUsed)
        mUsed = mPos;

    if (mPos > (signed long) mVector.size())
        mVector.resize(mPos);

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
    return mVector.size() - mPos;
}

void* CVectorOutStream::Data()
{
    return mVector.data();
}

void* CVectorOutStream::DataAtPosition()
{
    return mVector.data() + mPos;
}

void CVectorOutStream::Expand(unsigned long Amount)
{
    mVector.resize(mVector.size() + Amount);
}

void CVectorOutStream::Contract()
{
    mVector.resize(mUsed);
}

void CVectorOutStream::Reset()
{
    mPos = 0;
    mUsed = 0;
}

void CVectorOutStream::Clear()
{
    mVector.clear();
    Reset();
}
