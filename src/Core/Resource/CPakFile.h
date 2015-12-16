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
    u32 version;
    std::vector<SNamedResource> NamedResTable;
    std::vector<SResInfo> ResInfoTable;
    IInputStream* pak;

    bool decompress(u8 *src, u32 src_len, u8 *dst, u32 dst_len);

public:
    CPakFile();
    CPakFile(IInputStream* pakfile);
    ~CPakFile();

    std::vector<SNamedResource> getNamedResources();
    SResInfo getResourceInfo(u64 assetID, CFourCC assetType);
    std::vector<u8>* getResource(u64 assetID, CFourCC assetType);
    std::vector<u8>* getResource(SResInfo& info);
};

#endif // CPAKFILE_H
