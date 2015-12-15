#include "CSectionMgrOut.h"

CSectionMgrOut::CSectionMgrOut()
{
    mSectionCount = 0;
    mCurSectionStart = 0;
    mCurSectionIndex = 0;
}

void CSectionMgrOut::SetSectionCount(u32 Count)
{
    mSectionCount = Count;
    mSectionSizes.resize(Count);
}

void CSectionMgrOut::Init(const COutputStream& OutputStream)
{
    mCurSectionStart = OutputStream.Tell();
    mCurSectionIndex = 0;
}

void CSectionMgrOut::AddSize(COutputStream& OutputStream)
{
    mSectionSizes[mCurSectionIndex] = OutputStream.Tell() - mCurSectionStart;
    mCurSectionIndex++;
    mCurSectionStart = OutputStream.Tell();
}

void CSectionMgrOut::WriteSizes(COutputStream& OutputStream)
{
    for (u32 iSec = 0; iSec < mSectionCount; iSec++)
        OutputStream.WriteLong(mSectionSizes[iSec]);
}
