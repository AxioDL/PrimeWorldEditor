#ifndef CSECTIONMGRIN_H
#define CSECTIONMGRIN_H

#include <FileIO/IInputStream.h>
#include <Common/types.h>
#include <vector>

// The purpose of this class is to keep track of data block navigation - required to read CMDL and MREA files correctly
class CSectionMgrIn
{
    IInputStream *mpInputStream;
    std::vector<u32> mSectionSizes;
    u32 mCurSec;
    u32 mCurSecStart;
    u32 mSecsStart;

public:
    CSectionMgrIn(u32 Count, IInputStream* pSrc)
        : mpInputStream(pSrc)
    {
        mSectionSizes.resize(Count);

        for (u32 iSec = 0; iSec < Count; iSec++)
            mSectionSizes[iSec] = pSrc->ReadLong();
    }

    inline void Init()
    {
        // Initializes the block manager and marks the start of the first block
        mCurSec = 0;
        mCurSecStart = mpInputStream->Tell();
        mSecsStart = mCurSecStart;
    }

    void ToSection(u32 SecNum)
    {
        u32 Offset = mSecsStart;
        for (u32 iSec = 0; iSec < SecNum; iSec++)
            Offset += mSectionSizes[iSec];

        mpInputStream->Seek(Offset, SEEK_SET);
        mCurSec = SecNum;
        mCurSecStart = mpInputStream->Tell();
    }

    inline void ToNextSection()
    {
        mCurSecStart += mSectionSizes[mCurSec];
        mpInputStream->Seek(mCurSecStart, SEEK_SET);
        mCurSec++;
    }

    inline u32 NextOffset() { return mCurSecStart + mSectionSizes[mCurSec]; }
    inline u32 CurrentSection() { return mCurSec; }
    inline u32 CurrentSectionSize() { return mSectionSizes[mCurSec]; }
    inline u32 NumSections() { return mSectionSizes.size(); }
    inline void SetInputStream(IInputStream *pIn) { mpInputStream = pIn; }
};

#endif // CSECTIONMGRIN_H
