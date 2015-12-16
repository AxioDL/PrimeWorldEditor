#ifndef CBLOCKMGROUT_H
#define CBLOCKMGROUT_H

#include <Common/types.h>
#include <FileIO/IOutputStream.h>
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
    void Init(const IOutputStream& OutputStream);
    void AddSize(IOutputStream& OutputStream);
    void WriteSizes(IOutputStream& OutputStream);
};

#endif // CBLOCKMGROUT_H
