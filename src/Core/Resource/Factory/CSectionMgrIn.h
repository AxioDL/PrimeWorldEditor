#ifndef CSECTIONMGRIN_H
#define CSECTIONMGRIN_H

#include <Common/BasicTypes.h>
#include <Common/FileIO/IInputStream.h>
#include <vector>

// The purpose of this class is to keep track of data block navigation - required to read CMDL and MREA files correctly
class CSectionMgrIn
{
    IInputStream *mpInputStream;
    std::vector<uint32> mSectionSizes;
    uint32 mCurSec;
    uint32 mCurSecStart;
    uint32 mSecsStart;

public:
    CSectionMgrIn(uint32 Count, IInputStream* pSrc)
        : mpInputStream(pSrc)
    {
        mSectionSizes.resize(Count);

        for (uint32 iSec = 0; iSec < Count; iSec++)
            mSectionSizes[iSec] = pSrc->ReadLong();
    }

    inline void Init()
    {
        // Initializes the block manager and marks the start of the first block
        mCurSec = 0;
        mCurSecStart = mpInputStream->Tell();
        mSecsStart = mCurSecStart;
    }

    void ToSection(uint32 SecNum)
    {
        uint32 Offset = mSecsStart;
        for (uint32 iSec = 0; iSec < SecNum; iSec++)
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

    inline uint32 NextOffset()                      { return mCurSecStart + mSectionSizes[mCurSec]; }
    inline uint32 CurrentSection()                  { return mCurSec; }
    inline uint32 CurrentSectionSize()              { return mSectionSizes[mCurSec]; }
    inline uint32 NumSections()                     { return mSectionSizes.size(); }
    inline void SetInputStream(IInputStream *pIn)   { mpInputStream = pIn; }
};

#endif // CSECTIONMGRIN_H
