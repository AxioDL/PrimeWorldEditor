#ifndef SRESINFO_H
#define SRESINFO_H

#include <Common/CFourCC.h>
#include <Common/types.h>

struct SResInfo
{
    bool compressed;
    CFourCC resType;
    u64 resID;
    u32 offset;
    u32 size;
};

#endif // SRESINFO_H
