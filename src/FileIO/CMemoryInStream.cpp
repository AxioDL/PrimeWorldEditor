#include "CMemoryInStream.h"

CMemoryInStream::CMemoryInStream()
{
    mDataStart = nullptr;
    mDataSize = 0;
    mPos = 0;
}
CMemoryInStream::CMemoryInStream(void *Data, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    SetData(Data, Size, DataEndianness);
}

CMemoryInStream::~CMemoryInStream()
{
}

void CMemoryInStream::SetData(void *Data, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    mDataStart = static_cast<char*>(Data);
    mDataSize = Size;
    mPos = 0;
    mDataEndianness = DataEndianness;
}

void CMemoryInStream::ReadBytes(void *dst, unsigned long Count)
{
    if (!IsValid()) return;
    memcpy(dst, mDataStart + mPos, Count);
    mPos += Count;
}

bool CMemoryInStream::Seek(long Offset, long Origin)
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
            mPos = mDataSize - Offset;
            break;

        default:
            return false;
    }

    if (mPos < 0) {
        mPos = 0;
        return false;
    }

    if (mPos > mDataSize) {
        mPos = mDataSize;
        return false;
    }

    return true;
}

long CMemoryInStream::Tell() const
{
    return mPos;
}

bool CMemoryInStream::EoF() const
{
    return (mPos >= mDataSize);
}

bool CMemoryInStream::IsValid() const
{
    return (mDataStart != nullptr);
}

long CMemoryInStream::Size() const
{
    return mDataSize;
}

void CMemoryInStream::SetSize(unsigned long Size)
{
    mDataSize = Size;
    if (mPos > mDataSize)
        mPos = mDataSize;
}

void* CMemoryInStream::Data() const
{
    return mDataStart;
}

void* CMemoryInStream::DataAtPosition() const
{
    return mDataStart + mPos;
}
