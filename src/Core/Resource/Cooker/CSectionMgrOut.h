#ifndef CBLOCKMGROUT_H
#define CBLOCKMGROUT_H

#include <Common/BasicTypes.h>
#include <Common/FileIO/IOutputStream.h>
#include <vector>

// Small class to manage file sections for CMDL/MREA output
class CSectionMgrOut
{
    uint32 mSectionCount;
    uint32 mCurSectionStart;
    uint32 mCurSectionIndex;
    std::vector<uint32> mSectionSizes;

public:
    CSectionMgrOut()
        : mSectionCount(0)
        , mCurSectionStart(0)
        , mCurSectionIndex(0)
    {}

    void SetSectionCount(uint32 Count)
    {
        mSectionCount = Count;
        mSectionSizes.resize(Count);
    }

    void Init(const IOutputStream& rOut)
    {
        mCurSectionStart = rOut.Tell();
        mCurSectionIndex = 0;
    }

    void AddSize(IOutputStream& rOut)
    {
        mSectionSizes[mCurSectionIndex] = rOut.Tell() - mCurSectionStart;
        mCurSectionIndex++;
        mCurSectionStart = rOut.Tell();
    }

    void WriteSizes(IOutputStream& rOut)
    {
        for (uint32 iSec = 0; iSec < mSectionCount; iSec++)
            rOut.WriteLong(mSectionSizes[iSec]);
    }
};

#endif // CBLOCKMGROUT_H
