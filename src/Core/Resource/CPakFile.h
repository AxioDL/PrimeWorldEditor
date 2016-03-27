#ifndef CPAKFILE_H
#define CPAKFILE_H

#include "SNamedResource.h"
#include "SResInfo.h"
#include <Common/types.h>
#include <FileIO/CFileInStream.h>
#include <vector>

class CPakFile
{
private:
    u32 mVersion;
    std::vector<SNamedResource> mNamedResTable;
    std::vector<SResInfo> mResInfoTable;
    IInputStream* mpPak;

    bool Decompress(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen);

public:
    CPakFile();
    CPakFile(IInputStream* pPakFile);
    ~CPakFile();

    std::vector<SNamedResource> NamedResources();
    SResInfo ResourceInfo(u64 AssetID, CFourCC AssetType);
    std::vector<u8>* Resource(u64 AssetID, CFourCC AssetType);
    std::vector<u8>* Resource(SResInfo& rInfo);
};

#endif // CPAKFILE_H
