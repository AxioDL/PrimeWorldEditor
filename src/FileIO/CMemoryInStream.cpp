#include "CMemoryInStream.h"

CMemoryInStream::CMemoryInStream()
{
    mpDataStart = nullptr;
    mDataSize = 0;
    mPos = 0;
}
CMemoryInStream::CMemoryInStream(void *pData, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    SetData(pData, Size, DataEndianness);
}

CMemoryInStream::~CMemoryInStream()
{
}

void CMemoryInStream::SetData(void *pData, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    mpDataStart = static_cast<char*>(pData);
    mDataSize = Size;
    mPos = 0;
    mDataEndianness = DataEndianness;
}

void CMemoryInStream::ReadBytes(void *pDst, unsigned long Count)
{
    if (!IsValid()) return;
    memcpy(pDst, mpDataStart + mPos, Count);
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
    return (mpDataStart != nullptr);
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
    return mpDataStart;
}

void* CMemoryInStream::DataAtPosition() const
{
    return mpDataStart + mPos;
}
