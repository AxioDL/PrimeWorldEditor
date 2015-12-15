#ifndef CBLOCKMGRIN_H
#define CBLOCKMGRIN_H

#include <FileIO/CInputStream.h>
#include <vector>

// The purpose of this class is to keep track of data block navigation - required to read CMDL and MREA files correctly
class CBlockMgrIn
{
    CInputStream *mpInputStream;
    unsigned long mBlockCount;
    std::vector<unsigned long> mBlockSizes;
    unsigned long mCurBlock;
    unsigned long mCurBlockStart;
    unsigned long mBlocksStart;

public:
    CBlockMgrIn(unsigned long count, CInputStream* src);
    void Init();
    void ToBlock(unsigned long block);
    void ToNextBlock();
    long NextOffset();
    long CurrentBlock();
    long CurrentBlockSize();
    void SetInputStream(CInputStream *in);
};

#endif // CBLOCKMGRIN_H
