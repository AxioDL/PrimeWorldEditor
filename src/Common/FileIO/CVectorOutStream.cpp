#include "CVectorOutStream.h"
#include "Common/Common.h"

CVectorOutStream::CVectorOutStream()
    : mpVector(new std::vector<char>)
    , mOwnsVector(true)
    , mPos(0)
{
    mDataEndianness = IOUtil::eBigEndian;
}

CVectorOutStream::CVectorOutStream(IOUtil::EEndianness DataEndianness)
    : mpVector(new std::vector<char>)
    , mOwnsVector(true)
    , mPos(0)
{
    mDataEndianness = DataEndianness;
}

CVectorOutStream::CVectorOutStream(u32 InitialSize, IOUtil::EEndianness DataEndianness)
    : mpVector(new std::vector<char>(InitialSize))
    , mOwnsVector(true)
    , mPos(0)
{
    mDataEndianness = DataEndianness;
}

CVectorOutStream::CVectorOutStream(std::vector<char> *pVector, IOUtil::EEndianness DataEndianness)
    : mpVector(pVector)
    , mOwnsVector(false)
    , mPos(0)
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

    u32 NewSize = mPos + Count;

    if (NewSize > mpVector->size())
    {
        if (NewSize > mpVector->capacity())
            mpVector->reserve( ALIGN(mPos + Count, skAllocSize) );

        mpVector->resize(NewSize);
    }

    memcpy(mpVector->data() + mPos, pkSrc, Count);
    mPos += Count;
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
            mPos = mpVector->size() - Offset;
            break;

        default:
            return false;
    }

    if (mPos < 0)
    {
        mPos = 0;
        return false;
    }

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
    return mPos;
}

void CVectorOutStream::SetVector(std::vector<char> *pVector)
{
    if (mOwnsVector)
    {
        delete mpVector;
        mOwnsVector = false;
    }

    mpVector = pVector;
    mPos = 0;
}

void* CVectorOutStream::Data()
{
    return mpVector->data();
}

void* CVectorOutStream::DataAtPosition()
{
    return mpVector->data() + mPos;
}

void CVectorOutStream::Clear()
{
    mPos = 0;
    mpVector->clear();
}
