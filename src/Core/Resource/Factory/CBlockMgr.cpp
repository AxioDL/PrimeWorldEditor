#include "CBlockMgrIn.h"

CBlockMgrIn::CBlockMgrIn(unsigned long count, CInputStream* src)
{
    mpInputStream = src;
    mBlockCount = count;
    mBlockSizes.resize(count);

    for (unsigned long b = 0; b < count; b++)
        mBlockSizes[b] = src->ReadLong();
}

void CBlockMgrIn::Init()
{
    // Initialize the block manager; this marks the start of the first block
    mCurBlock = 0;
    mCurBlockStart = mpInputStream->Tell();
    mBlocksStart = mpInputStream->Tell();
}

void CBlockMgrIn::ToBlock(unsigned long block)
{
    unsigned long offset = mBlocksStart;
    for (unsigned long b = 0; b < block; b++)
        offset += mBlockSizes[b];

    mpInputStream->Seek(offset, SEEK_SET);

    mCurBlock = block;
    mCurBlockStart = mpInputStream->Tell();
}

void CBlockMgrIn::ToNextBlock()
{
    mpInputStream->Seek(mCurBlockStart + mBlockSizes[mCurBlock], SEEK_SET);
    mCurBlock++;
    mCurBlockStart = mpInputStream->Tell();
}

long CBlockMgrIn::NextOffset()
{
    return mCurBlockStart + mBlockSizes[mCurBlock];
}

long CBlockMgrIn::CurrentBlock()
{
    return mCurBlock;
}

long CBlockMgrIn::CurrentBlockSize()
{
    return mBlockSizes[mCurBlock];
}

void CBlockMgrIn::SetInputStream(CInputStream *in)
{
    mpInputStream = in;
}
