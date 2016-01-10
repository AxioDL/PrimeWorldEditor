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

    SResInfo()
        : compressed(false), resType("NULL"), resID(0), offset(0), size(0) {}
};

#endif // SRESINFO_H
