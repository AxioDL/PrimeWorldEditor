#ifndef CBLOCKMGROUT_H
#define CBLOCKMGROUT_H

#include <Common/types.h>
#include <FileIO/COutputStream.h>
#include <vector>

// Small class to manage file sections for CMDL/MREA output
class CSectionMgrOut
{
    u32 mSectionCount;
    u32 mCurSectionStart;
    u32 mCurSectionIndex;
    std::vector<u32> mSectionSizes;

public:
    CSectionMgrOut();
    void SetSectionCount(u32 Count);
    void Init(const COutputStream& OutputStream);
    void AddSize(COutputStream& OutputStream);
    void WriteSizes(COutputStream& OutputStream);
};

#endif // CBLOCKMGROUT_H
