#ifndef SRESINFO_H
#define SRESINFO_H

#include <Common/CFourCC.h>
#include <Common/types.h>

struct SResInfo
{
    bool Compressed;
    CFourCC Type;
    u64 ID;
    u32 Offset;
    u32 Size;

    SResInfo()
        : Compressed(false), Type("NULL"), ID(0), Offset(0), Size(0) {}
};

#endif // SRESINFO_H
