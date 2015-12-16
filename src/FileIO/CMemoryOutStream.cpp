#include "CMemoryOutStream.h"

CMemoryOutStream::CMemoryOutStream()
{
    mpDataStart = nullptr;
    mDataSize = 0;
    mPos = 0;
    mUsed = 0;
}

CMemoryOutStream::CMemoryOutStream(void *pData, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    SetData(pData, Size, DataEndianness);
}

CMemoryOutStream::~CMemoryOutStream()
{
}

void CMemoryOutStream::SetData(void *pData, unsigned long Size, IOUtil::EEndianness DataEndianness)
{
    mpDataStart = static_cast<char*>(pData);
    mDataSize = Size;
    mPos = 0;
    mUsed = 0;
    mDataEndianness = DataEndianness;
}

void CMemoryOutStream::WriteBytes(void *pSrc, unsigned long Count)
{
    if (!IsValid()) return;

    memcpy(mpDataStart + mPos, pSrc, Count);
    mPos += Count;
    if (mPos > mUsed) mUsed = mPos;
}

bool CMemoryOutStream::Seek(long Offset, long Origin)
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

long CMemoryOutStream::Tell() const
{
    return mPos;
}

bool CMemoryOutStream::EoF() const
{
    return (mPos >= mDataSize);
}

bool CMemoryOutStream::IsValid() const
{
    return (mpDataStart != nullptr);
}

long CMemoryOutStream::Size() const
{
    return mDataSize;
}

long CMemoryOutStream::SpaceUsed() const
{
    return mUsed;
}

void CMemoryOutStream::SetSize(unsigned long Size)
{
    mDataSize = Size;
    if (mPos > mDataSize)
        mPos = mDataSize;
}

void* CMemoryOutStream::Data() const
{
    return mpDataStart;
}

void* CMemoryOutStream::DataAtPosition() const
{
    return mpDataStart + mPos;
}
