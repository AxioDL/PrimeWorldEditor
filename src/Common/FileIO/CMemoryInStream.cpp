#include "CMemoryInStream.h"

CMemoryInStream::CMemoryInStream()
    : mpDataStart(nullptr)
    , mDataSize(0)
    , mPos(0)
{
}
CMemoryInStream::CMemoryInStream(const void *pkData, u32 Size, IOUtil::EEndianness DataEndianness)
{
    SetData(pkData, Size, DataEndianness);
}

CMemoryInStream::~CMemoryInStream()
{
}

void CMemoryInStream::SetData(const void *pkData, u32 Size, IOUtil::EEndianness DataEndianness)
{
    mpDataStart = (const char*) pkData;
    mDataSize = Size;
    mPos = 0;
    mDataEndianness = DataEndianness;
}

void CMemoryInStream::ReadBytes(void *pDst, u32 Count)
{
    if (!IsValid()) return;
    memcpy(pDst, mpDataStart + mPos, Count);
    mPos += Count;
}

bool CMemoryInStream::Seek(s32 Offset, u32 Origin)
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

    if (mPos < 0)
    {
        mPos = 0;
        return false;
    }

    if (mPos > mDataSize)
    {
        mPos = mDataSize;
        return false;
    }

    return true;
}

u32 CMemoryInStream::Tell() const
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

u32 CMemoryInStream::Size() const
{
    return mDataSize;
}

void CMemoryInStream::SetSize(u32 Size)
{
    mDataSize = Size;
    if (mPos > mDataSize)
        mPos = mDataSize;
}

const void* CMemoryInStream::Data() const
{
    return mpDataStart;
}

const void* CMemoryInStream::DataAtPosition() const
{
    return mpDataStart + mPos;
}
