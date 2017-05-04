#include "CBitStreamInWrapper.h"

CBitStreamInWrapper::CBitStreamInWrapper(IInputStream *pStream, EChunkSize ChunkSize /*= e32Bit*/)
    : mpSourceStream(pStream)
    , mChunkSize(ChunkSize)
    , mBitPool(0)
    , mBitsRemaining(0)
{
}

void CBitStreamInWrapper::SetChunkSize(EChunkSize Size)
{
    mChunkSize = Size;
}

long CBitStreamInWrapper::ReadBits(u32 NumBits, bool ExtendSignBit /*= true*/)
{
    u32 BitsRemaining = NumBits;
    u32 Out = 0;
    u32 Shift = 0;

    while (BitsRemaining > 0)
    {
        if (mBitsRemaining < BitsRemaining)
        {
            BitsRemaining -= mBitsRemaining;
            Out |= (mBitPool << Shift);
            Shift += mBitsRemaining;
            ReplenishPool();
        }

        else
        {
            long Mask = (1 << BitsRemaining) - 1;
            Out |= (mBitPool & Mask) << Shift;
            mBitPool >>= BitsRemaining;
            mBitsRemaining -= BitsRemaining;
            BitsRemaining = 0;
        }
    }

    if (ExtendSignBit)
    {
        bool Sign = ((Out >> (NumBits - 1) & 0x1) == 1);
        if (Sign) Out |= (-1 << NumBits);
    }

    return Out;
}

bool CBitStreamInWrapper::ReadBit()
{
    return (ReadBits(1, false) != 0);
}

// ************ PRIVATE ************
void CBitStreamInWrapper::ReplenishPool()
{
    if (mChunkSize == e8Bit)
        mBitPool = mpSourceStream->ReadByte();
    else if (mChunkSize == e16Bit)
        mBitPool = mpSourceStream->ReadShort();
    else if (mChunkSize == e32Bit)
        mBitPool = mpSourceStream->ReadLong();

    mBitsRemaining = mChunkSize;
}
