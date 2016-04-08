#ifndef CBITSTREAMINWRAPPER_H
#define CBITSTREAMINWRAPPER_H

#include "IInputStream.h"

class CBitStreamInWrapper
{
public:
    enum EChunkSize
    {
        e8Bit = 8, e16Bit = 16, e32Bit = 32
    };

private:
    IInputStream *mpSourceStream;
    EChunkSize mChunkSize;
    unsigned long mBitPool;
    long mBitsRemaining;

public:
    CBitStreamInWrapper(IInputStream *pStream, EChunkSize ChunkSize = e32Bit);
    void SetChunkSize(EChunkSize Size);
    long ReadBits(long NumBits, bool ExtendSignBit = true);
    bool ReadBit();

private:
    void ReplenishPool();
};

#endif // CBITSTREAMINWRAPPER_H
